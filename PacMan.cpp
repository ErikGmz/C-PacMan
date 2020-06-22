#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum lista { abajo = 0, arriba = 4, izquierda = 8, derecha = 12 };

struct Datos {
    char nombre[7];
    char* codigo;
    int puntaje, nivel, vidas, mapa_actual;
};

struct Movimiento {
    int coord_x, coord_y, animacion; 
    bool continuar_nivel;
    char nombre_archivo[30],  numero[3], cifra[9], impresion_nivel[3];
    lista direccion = abajo, direccion_previa = abajo;
};

union Mapas {
    char mapa[101][101];
};

void inicializar_graficos();
void comenzar_juego(ALLEGRO_DISPLAY*, ALLEGRO_SAMPLE*, ALLEGRO_SAMPLE_ID, FILE*);
Datos preguntar_nombre(ALLEGRO_DISPLAY*);
char convertir_letra(ALLEGRO_EVENT);
Mapas llenar_mapa1();
Mapas llenar_mapa2();
bool movimiento_pacman(char* mapa[], Datos& jugador, Movimiento& juego, int velocidad);
char* generar_codigo();
char letras_aleatorias();
void continuar();
void checar_records(ALLEGRO_DISPLAY*, FILE*);

int main(int argc, char** argv) {
    srand(time(NULL));
    inicializar_graficos();
    bool salir = false, empezar = false, sonido = true, reanudar;
    int x = 0, y = 0;
    char* codigo = generar_codigo();

    FILE* registro = fopen("registro.dat", "rb");
    if (!registro) registro = fopen("registro.dat", "ab");
    fclose(registro);
    
    ALLEGRO_DISPLAY* pantalla = al_create_display(600, 600);
    ALLEGRO_SAMPLE* opcion = al_load_sample("sounds/selection.mp3");
    ALLEGRO_SAMPLE* click = al_load_sample("sounds/click.mp3");
    ALLEGRO_SAMPLE* OST = al_load_sample("sounds/Mega Man 3 (NES) Music Title Theme.mp3");
    ALLEGRO_SAMPLE_ID id;
    ALLEGRO_BITMAP* pantalla_inicio = al_load_bitmap("img/pantInicioa.png");
    ALLEGRO_BITMAP* icono = al_load_bitmap("img/sprites/fruits/cherry.png");
    ALLEGRO_BITMAP* boton_A = al_load_bitmap("img/presiona.png");
    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_BITMAP* opcion1 = al_load_bitmap("img/iniciar.png");
    ALLEGRO_BITMAP* opcion2 = al_load_bitmap("img/continuar.png");
    ALLEGRO_BITMAP* opcion3 = al_load_bitmap("img/records.png");
    ALLEGRO_BITMAP* opcion4 = al_load_bitmap("img/salir.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();

    al_reserve_samples(3);
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_mouse_event_source());
    al_register_event_source(fila_evento, al_get_display_event_source(pantalla));
    al_set_window_title(pantalla, "PacMan");
    al_set_display_icon(pantalla, icono);

    al_draw_bitmap(pantalla_inicio, 13, 30, NULL);
    al_draw_bitmap(boton_A, 232, 380, NULL);
    al_flip_display();

    al_play_sample(OST, 0.4, 0, 1.0, ALLEGRO_PLAYMODE_LOOP, &id);

    while (!empezar) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch(evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if (evento.keyboard.keycode == ALLEGRO_KEY_A) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                empezar = true;

                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);
            }
            break;
        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
            reanudar = false;
            while (!reanudar) {
                ALLEGRO_EVENT evento2;
                al_wait_for_event(fila_evento, &evento2);

                if (evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                    al_draw_bitmap(pantalla_inicio, 13, 30, NULL);
                    al_draw_bitmap(boton_A, 232, 380, NULL);
                    al_flip_display();
                    reanudar = true;
                }
            }
            break;
        }
    }

    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(menu, 13, 30, NULL);
        al_draw_bitmap(opcion1, 232, 280, NULL);
        al_draw_bitmap(opcion2, 212, 340, NULL);
        al_draw_bitmap(opcion3, 225, 400, NULL);
        al_draw_bitmap(opcion4, 249, 460, NULL);
        al_flip_display();

        switch (evento.type) {
        case ALLEGRO_EVENT_MOUSE_AXES:
            x = evento.mouse.x;
            y = evento.mouse.y;
            if (x > 232 && x < 360 && y > 280 && y < 312 || x > 212 && x < 388 && y > 340 && y < 372 || x > 225 && x < 373 && y > 400 && y < 432 || x > 249 && x < 343 && y > 460 && y < 492) {
                if (!sonido) {
                    sonido = true;
                    al_play_sample(opcion, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                }
            }
            else {
                sonido = false;
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (x > 232 && x < 360 && y > 280 && y < 312) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);
                comenzar_juego(pantalla, OST, id, registro);
            }
            if (x > 212 && x < 388 && y > 340 && y < 372) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);
                continuar();
            }
            if (x > 225 && x < 373 && y > 400 && y < 432) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);
                checar_records(pantalla, registro);
            }
            if (x > 249 && x < 343 && y > 460 && y < 492) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                salir = true;
            }
            break;
        }
    }
        
    /*al_destroy_display(pantalla); al_destroy_font(formato); al_destroy_bitmap(pantalla_inicio);
    al_destroy_bitmap(boton_A); al_destroy_bitmap(menu); al_destroy_bitmap(opcion1);
    al_destroy_bitmap(opcion2); al_destroy_bitmap(opcion3); al_destroy_bitmap(opcion4);
    al_destroy_sample(OST); al_destroy_sample(opcion); al_destroy_sample(click);*/

    return 0;
}
    
void inicializar_graficos() {
    al_init();
    al_install_audio();
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_init_acodec_addon();
}

void comenzar_juego(ALLEGRO_DISPLAY* pantalla, ALLEGRO_SAMPLE* OST, ALLEGRO_SAMPLE_ID id, FILE* registro) {
    Datos jugador, auxiliar;
    Movimiento juego;
    Mapas tablero;
    char* mapa[111];
    bool finalizado = false, nombre_encontrado = false, vida_perdida, reanudar;
    const int velocidad = 5;
    int puntuacion_previa, continuar_juego, continuar = false;

    FILE* codigos = fopen("constraseñas.dat", "rb+");
    if (!codigos) {
        codigos = fopen("constraseñas.dat", "ab");
        fclose(codigos);
        codigos = fopen("constraseñas.dat", "rb+");
    }

    ALLEGRO_SAMPLE* fin_nivel = al_load_sample("sounds/07 - Round Clear.mp3");
    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_FONT* formato1 = al_load_ttf_font("fonts/04B_30__.ttf", 18, NULL);
    ALLEGRO_FONT* formato2 = al_load_ttf_font("fonts/04B_30__.ttf", 15, NULL);
    ALLEGRO_FONT* formato3 = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);
    al_register_event_source(fila_evento, al_get_keyboard_event_source());

    registro = fopen("registro.dat", "rb+");
    jugador = preguntar_nombre(pantalla);
    jugador.nivel = 1; jugador.vidas = 3;
    jugador.mapa_actual = 1;

    al_stop_sample(&id);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    while (!finalizado) {
        switch (jugador.mapa_actual) {
        case 1:
            juego.coord_y = 347;
            tablero = llenar_mapa1(); 
            break;
        case 2: 
            juego.coord_y = 363;
            tablero = llenar_mapa2(); 
            break;
        }
        juego.coord_x = 287;
        juego.animacion = 0;
        juego.continuar_nivel = true;
        juego.direccion = abajo;
        juego.direccion_previa = abajo;
        jugador.codigo = new char[7];
        vida_perdida = false;
        continuar_juego = 0;

        for (int i = 0; i < 111; i++) {
            *(mapa + i) = tablero.mapa[i];;
        }

        puntuacion_previa = jugador.puntaje;
        vida_perdida = movimiento_pacman(mapa, jugador, juego, velocidad);

        if (!vida_perdida) {
            jugador.nivel++;
            al_play_sample(fin_nivel, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

            al_rest(2.5);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);

            al_draw_bitmap(menu, 13, 30, NULL);
            al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "NIVEL FINALIZADO");
            al_draw_text(formato2, al_map_rgb(255, 255, 39), 160, 340, NULL, "SELECCIONE UNA OPCION");
            al_draw_text(formato2, al_map_rgb(255, 255, 39), 165, 420, NULL, "CONTINUAR");
            al_draw_text(formato3, al_map_rgb(255, 255, 39), 180, 450, NULL, "TECLA (A)");
            al_draw_text(formato2, al_map_rgb(255, 255, 39), 320, 420, NULL, "FINALIZAR");
            al_draw_text(formato3, al_map_rgb(255, 255, 39), 330, 450, NULL, "TECLA (S)");
            al_flip_display();

            while (continuar_juego != 1 && continuar_juego != 2) {
                ALLEGRO_EVENT evento;
                al_wait_for_event(fila_evento, &evento);

                switch (evento.type) {
                case ALLEGRO_EVENT_KEY_DOWN: 
                    if (evento.keyboard.keycode == ALLEGRO_KEY_A) continuar_juego = 1;
                    if (evento.keyboard.keycode == ALLEGRO_KEY_S) continuar_juego = 2;
                    break;
                case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT: 
                    reanudar = false;
                    while (!reanudar) {
                        ALLEGRO_EVENT evento2;
                        al_wait_for_event(fila_evento, &evento2);

                        if (evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {

                            al_draw_bitmap(menu, 13, 30, NULL);
                            al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "NIVEL FINALIZADO");
                            al_draw_text(formato2, al_map_rgb(255, 255, 39), 160, 340, NULL, "SELECCIONE UNA OPCION");
                            al_draw_text(formato2, al_map_rgb(255, 255, 39), 165, 420, NULL, "CONTINUAR");
                            al_draw_text(formato3, al_map_rgb(255, 255, 39), 180, 450, NULL, "TECLA (A)");
                            al_draw_text(formato2, al_map_rgb(255, 255, 39), 320, 420, NULL, "FINALIZAR");
                            al_draw_text(formato3, al_map_rgb(255, 255, 39), 330, 450, NULL, "TECLA (S)");
                            al_flip_display();
                            reanudar = true;
                        }
                    }
                    break;
                }
            }
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);
        }
        else {
            jugador.puntaje = puntuacion_previa;
            al_rest(1.5);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);
        }
                       
        (jugador.nivel % 2 == 0) ? jugador.mapa_actual = 2 : jugador.mapa_actual = 1;

        if (jugador.vidas == 0 || jugador.nivel == 11 || continuar_juego == 2) finalizado = true;
    }

    if (jugador.vidas > 0 && jugador.nivel != 11) {
        jugador.codigo = generar_codigo();
    }
    
    al_draw_bitmap(menu, 13, 30, NULL);
    al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "JUEGO FINALIZADO");
    al_draw_text(formato2, al_map_rgb(255, 255, 39), 187, 350, NULL, "PRESIONE LA TECLA");
    al_draw_text(formato2, al_map_rgb(255, 255, 39), 285, 370, NULL, "(Q)");
    if (jugador.vidas > 0 && jugador.nivel != 11) {
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 213, 420, NULL, "LA CLAVE PARA");
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 170, 440, NULL, "REANUDAR LA PARTIDA");
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 288, 460, NULL, "ES");
        al_draw_text(formato2, al_map_rgb(255, 163, 1), 260, 490, NULL, jugador.codigo);
    }
    al_flip_display();

    while (!continuar) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch (evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if (evento.keyboard.keycode == ALLEGRO_KEY_Q) {
                continuar = true;
            }
            break;
        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
            reanudar = false;
            while (!reanudar) {
                ALLEGRO_EVENT evento2;
                al_wait_for_event(fila_evento, &evento2);

                if (evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "JUEGO FINALIZADO");
                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 187, 350, NULL, "PRESIONE LA TECLA");
                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 285, 370, NULL, "(Q)");
                    if (jugador.vidas > 0 && jugador.nivel != 11) {
                        al_draw_text(formato2, al_map_rgb(255, 255, 39), 213, 420, NULL, "LA CLAVE PARA");
                        al_draw_text(formato2, al_map_rgb(255, 255, 39), 170, 440, NULL, "REANUDAR LA PARTIDA");
                        al_draw_text(formato2, al_map_rgb(255, 255, 39), 288, 460, NULL, "ES");
                        al_draw_text(formato2, al_map_rgb(255, 163, 1), 260, 490, NULL, jugador.codigo);
                    }
                    al_flip_display();
                    reanudar = true;
                }
            }
            break;
        }
    }

    al_rest(0.3);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_rest(0.3);

    if (jugador.vidas > 0 && jugador.nivel != 11) {
        fseek(codigos, 0, SEEK_END);
        if (ftell(codigos) == 0) {
            fwrite(&jugador, sizeof(Datos), 1, codigos);
        }
        else {
            fseek(codigos, 0, SEEK_SET);
            fread(&auxiliar, sizeof(Datos), 1, codigos);
            while (!feof(codigos)) {
                if (strcmp(auxiliar.nombre, jugador.nombre) == 0) {
                    nombre_encontrado = true;
                    fseek(codigos, ftell(codigos) - sizeof(Datos), SEEK_SET);
                    fwrite(&jugador, sizeof(Datos), 1, codigos);
                    break;
                }
                fread(&auxiliar, sizeof(Datos), 1, codigos);
            }

            if (!nombre_encontrado) {
                fseek(codigos, 0, SEEK_END);
                fwrite(&jugador, sizeof(Datos), 1, codigos);
            }
        }
    }
    rewind(codigos);
    fclose(codigos);
    nombre_encontrado = false;

    fseek(registro, 0, SEEK_END);
    if (ftell(registro) == 0) {
        fwrite(&jugador, sizeof(Datos), 1, registro);
    }
    else {
        fseek(registro, 0, SEEK_SET);
        fread(&auxiliar, sizeof(Datos), 1, registro);
        while (!feof(registro)) {
            if (strcmp(auxiliar.nombre, jugador.nombre) == 0) {
                nombre_encontrado = true;
                if (jugador.puntaje > auxiliar.puntaje) {
                    fseek(registro, ftell(registro) - sizeof(Datos), SEEK_SET);
                    fwrite(&jugador, sizeof(Datos), 1, registro);
                }
                break;
            }
            fread(&auxiliar, sizeof(Datos), 1, registro);
        }

        if (!nombre_encontrado) {
            fseek(registro, 0, SEEK_END);
            fwrite(&jugador, sizeof(Datos), 1, registro);
        }
    }
    rewind(registro);
    fclose(registro);
}

Datos preguntar_nombre(ALLEGRO_DISPLAY* pantalla) {
    Datos jugador;
    bool finalizado = false, reanudar;
    char cadena[7] = "", auxiliar;
    int contador = 0;

    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_SAMPLE* tecla = al_load_sample("sounds/selection.mp3");
    ALLEGRO_SAMPLE* click = al_load_sample("sounds/click.mp3");
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 18, NULL);
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();

    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_display_event_source(pantalla));

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(menu, 13, 30, NULL);
    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
    al_flip_display();
    
    while (!finalizado) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch(evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if(evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if(strlen(cadena) == 0){
                    al_show_native_message_box(pantalla, "Advertencia", "Error de formato", "Nombre mal introducido", NULL, ALLEGRO_MESSAGEBOX_WARN);
                }
                else {
                    al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    finalizado = true;
                }
            }
            else {
                auxiliar = convertir_letra(evento);
                if(auxiliar != '+' && strlen(cadena) < 6) al_play_sample(tecla, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                fflush(stdin);
                if (auxiliar != '+' && auxiliar != '-' && contador < 6) {
                    cadena[contador] = auxiliar;

                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
                    switch (strlen(cadena)) {
                    case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena); break;
                    case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena); break;
                    case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena); break;
                    case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena); break;
                    case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena); break;
                    case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena); break;
                    }
                    al_flip_display();

                    contador++;
                }
                if (auxiliar == '-' && contador >= 0) {
                    if (contador > 0) contador--;
                    cadena[contador] = NULL;

                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
                    switch (strlen(cadena)) {
                    case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena); break;
                    case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena); break;
                    case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena); break;
                    case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena); break;
                    case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena); break;
                    case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena); break;
                    }
                    al_flip_display();
                }
                fflush(stdin);
            }
            al_flip_display();
            break;
        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
            reanudar = false;
            while(!reanudar) {
                ALLEGRO_EVENT evento2;
                al_wait_for_event(fila_evento, &evento2);

                if(evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
                    switch (strlen(cadena)) {
                    case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena); break;
                    case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena); break;
                    case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena); break;
                    case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena); break;
                    case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena); break;
                    case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena); break;
                    }
                    al_flip_display();
                    reanudar = true;
                }
            }
            break;
        }
    }

    al_rest(0.3);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_rest(0.5);

    jugador.puntaje = 0;
    strcpy_s(jugador.nombre, 7, cadena);

    return jugador;
}

char convertir_letra(ALLEGRO_EVENT evento) {
    char letra;
    switch (evento.keyboard.keycode) {
    case ALLEGRO_KEY_A: letra = 'A'; break; case ALLEGRO_KEY_B: letra = 'B'; break; case ALLEGRO_KEY_C: letra = 'C'; break; 
    case ALLEGRO_KEY_D: letra = 'D'; break; case ALLEGRO_KEY_E: letra = 'E'; break; case ALLEGRO_KEY_F: letra = 'F'; break;
    case ALLEGRO_KEY_G: letra = 'G'; break; case ALLEGRO_KEY_H: letra = 'H'; break; case ALLEGRO_KEY_I: letra = 'I'; break; 
    case ALLEGRO_KEY_J: letra = 'J'; break; case ALLEGRO_KEY_K: letra = 'K'; break; case ALLEGRO_KEY_L: letra = 'L'; break;
    case ALLEGRO_KEY_M: letra = 'M'; break; case ALLEGRO_KEY_N: letra = 'N'; break; case ALLEGRO_KEY_O: letra = 'O'; break; 
    case ALLEGRO_KEY_P: letra = 'P'; break; case ALLEGRO_KEY_Q: letra = 'Q'; break; case ALLEGRO_KEY_R: letra = 'R'; break;
    case ALLEGRO_KEY_S: letra = 'S'; break; case ALLEGRO_KEY_T: letra = 'T'; break; case ALLEGRO_KEY_U: letra = 'U'; break;
    case ALLEGRO_KEY_V: letra = 'V'; break; case ALLEGRO_KEY_W: letra = 'W'; break; case ALLEGRO_KEY_X: letra = 'X'; break;
    case ALLEGRO_KEY_Y: letra = 'Y'; break; case ALLEGRO_KEY_Z: letra = 'Z'; break; case ALLEGRO_KEY_1: letra = '1'; break;
    case ALLEGRO_KEY_2: letra = '2'; break; case ALLEGRO_KEY_3: letra = '3'; break; case ALLEGRO_KEY_4: letra = '4'; break;
    case ALLEGRO_KEY_5: letra = '5'; break; case ALLEGRO_KEY_6: letra = '6'; break; case ALLEGRO_KEY_7: letra = '7'; break;
    case ALLEGRO_KEY_8: letra = '8'; break; case ALLEGRO_KEY_9: letra = '9'; break; case ALLEGRO_KEY_0: letra = '0'; break;
    case ALLEGRO_KEY_BACKSPACE: letra = '-'; break; default: letra = '+'; break;
    }
    return letra;
}

Mapas llenar_mapa1() {
    Mapas tablero;
    char mapa[101][101] =
    {"     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     ***o    o    o   o   o   o    o    o   o********o   o    o    o   o   o   o    o    o******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     ***o*********o****************o**************************o****************o*********o******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     ***o*********o****************o**************************o****************o*********o******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     *** ********* **************** ************************** **************** ********* ******    ",
     "     ***o    o    o   o   o   o    o    o   o    o   o   o    o    o   o   o   o    o    o******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     ***o***********************************o********o***********************************o******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     ***o*********o     o******************* ******** *******************o     o*********o******    ",
     "     ******************* *********o    o    o    o   o    o    o********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     *******************o*********o     ******************     o*********o**********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     ***o    o    o                            o    o    o*****                     ",
     "                     *** ********* ************** ************* ********* *****                     ",
     "                     *** ********* ************** ************* ********* *****                     ",
     "                     *** ********* ************** ************* ********* *****                     ",
     "                     *** ********* ************** ************* ********* *****                     ",
     "                     ***o********* ************** ************* *********o*****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "************************ ********* **************************** ********* **************************",
     "************************ ********* ****                   ***** ********* **************************",
     "************************ ********* ****                   ***** ********* **************************",
     "*       o   o   o   o   o          ****                   *****          o   o   o   o   o         *",
     "************************ ********* ****                   ***** ********* **************************",
     "************************ ********* ****                   ***** ********* **************************",
     "     ******************* ********* ****                   ***** ********* **********************    ",
     "     ******************* ********* ****                   ***** ********* **********************    ",
     "     *******************o********* ****                   ***** *********o**********************    ",
     "     ******************* ********* ****                   ***** ********* **********************    ",
     "     ******************* ********* **************************** ********* **********************    ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** *********                              ********* *****                     ",
     "                     ***o********* **************************** *********o*****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** *********o*****                     ",
     "                     ***o********* **************************** ********* *****                     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ***o    o    o     o    o    o    o    o********o    o    o   o     o     o    o    o*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o***************o*******************o********o*******************o***************o*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o     o*********o    o    o    o    o   o    o    o    o   o     o*********o     o*****     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     *********o*********o***********************o************************o*********o***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ***o     o*********o*********o    o    o   o    o   o    o**********o*********o     o*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o***************o*******************o********o*******************o***************o*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o     o    o    o    o    o    o    o   o    o     o   o   o     o    o    o     o*****     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
    };

    for (int i = 0; i < 101; i++) {
        strcpy_s(*(tablero.mapa + i), 101, mapa[i]);
    }
    return tablero;
}

Mapas llenar_mapa2() {
    Mapas tablero;
    char mapa[101][101] =
    {"     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ***o    o    o    o   o    o   o   o   o*********o   o   o   o   o   o    o    o    o*****     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ******************o************o*************************o***********o********************     ",
     "     ****************** ************ ************************* *********** ********************    ",
     "*********************** ************ ************************* *********** *************************",
     "*********************** ************ ************************* *********** *************************",
     "*      o   o   o   o   o************o*************************o***********o   o   o   o   o        *",
     "*********************** ************ ************************* *********** *************************",
     "*********************** ************ ************************* *********** *************************",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ******************o************o*************************o***********o********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ***o    o    o    o   o    o   o   o   o    o   o    o   o   o   o   o    o    o    o*****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     ***o***********************************o********o***********************************o*****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********o     o******************* ******** *******************o     o********* *****     ",
     "     ***o*************** *********o    o    o    o   o    o    o********* ***************o*****     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     *******************o*********o     ******************     o*********o*********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ***o   o   o   o   o    o    o                            o    o    o   o   o   o   o*****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     ***o***************o********* ************* ************** *********o***************o*****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     *** *************** ********* ************* ************** ********* *************** *****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     ***o     o*********o          ****                   *****          o*********o     o*****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     ***o***************o********* ****                   ***** *********o***************o*****     ",
     "     *** *************** ********* ****                   ***** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     ***o   o   o   o   o*********                              *********o   o   o   o   o*****     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     *******************o********* **************************** *********o*********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     *** **********     o    o    o    o    o*********o    o   o    o    o       ******** *****     ",
     "     *** ********** **** ******************* ********* ****************** ****** ******** *****     ",
     "     *** ********** **** ******************* ********* ****************** ****** ******** *****     ",
     "     *** ********** **** ******************* ********* ****************** ****** ******** *****     ",
     "     *** ********** **** ******************* ********* ****************** ****** ******** *****     ",
     "     *** ********** **** *******************o*********o****************** ****** ******** *****     ",
     "     *** ********** **** ******************* ********* ****************** ****** ******** *****     ",
     "     ***                 ******************* ********* ******************                 *****     ",
     "     ******************* ******************* ********* ****************** *********************     ",
     "     ******************* ******************* ********* ****************** *********************     ",
     "     *******************o    o    o    o    o*********o    o   o    o    o*********************     ",
     "     ******************* ************************************************ *********************     ",
     "     ******************* ************************************************ *********************     ",
     "************************ ************************************************ **************************",
     "************************ ************************************************ **************************",
     "*                       o************************************************o                         *",
     "************************ ************************************************ **************************",
     "************************ ************************************************ **************************",
     "     ******************* ************************************************ *********************     ",
     "     ******************* *********o    o    o********o    o    o********* *********************     ",
     "     ******************* ******************* ******** ******************* *********************     ",
     "     ******************* ******************* ******** ******************* *********************     ",
     "     ***o     o    o    o******************* ******** *******************o     o    o    o*****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** *******************o********o******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     ***o**********o    o    o    o    o    o    o   o    o    o   o     o     o*********o*****     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
    };

    for (int i = 0; i < 101; i++) {
        strcpy_s(*(tablero.mapa + i), 101, mapa[i]);
    }
    return tablero;
}

bool movimiento_pacman(char *mapa[], Datos &jugador, Movimiento &juego, int velocidad) {
    ALLEGRO_BITMAP* pacman[28];
    ALLEGRO_BITMAP* mapa1 = al_load_bitmap("img/mapa1B.png");
    ALLEGRO_BITMAP* mapa2 = al_load_bitmap("img/mapa2.png");
    ALLEGRO_BITMAP* pac_dot = al_load_bitmap("img/sprites/map/pill.png");
    ALLEGRO_BITMAP* negro = al_load_bitmap("img/negro.png");
    ALLEGRO_BITMAP* icono = al_load_bitmap("img/sprites/ui/life.png");
    ALLEGRO_BITMAP* marcador = al_load_bitmap("img/pacman/ready.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_TIMER* temporizador = al_create_timer(1.0 / 15);
    ALLEGRO_KEYBOARD_STATE estado_tecla;
    ALLEGRO_SAMPLE* comer = al_load_sample("sounds/waka.mp3");
    ALLEGRO_SAMPLE* morir = al_load_sample("sounds/pacmandying.mp3");
    ALLEGRO_SAMPLE* intro = al_load_sample("sounds/intro.mp3");
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);
    bool indicador, movimiento_activado = true, juego_iniciado = false;
    int contador = 15, inicializacion = 0;

    al_register_event_source(fila_evento, al_get_timer_event_source(temporizador));
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_start_timer(temporizador);

    for (int i = 0; i < 28; i++) {
        strcpy_s(juego.nombre_archivo, 30, "img/pacman/pacman");
        sprintf_s(juego.numero, 3, "%i", i + 1);
        strcat_s(juego.nombre_archivo, 30, juego.numero);
        strcat_s(juego.nombre_archivo, 30, ".png");
        pacman[i] = al_load_bitmap(juego.nombre_archivo);
        al_convert_mask_to_alpha(pacman[i], al_map_rgb(0, 0, 0));
    }

    while (juego.continuar_nivel) {
        indicador = false;
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        if(evento.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&estado_tecla);
            juego.direccion_previa = juego.direccion;

            if (al_key_down(&estado_tecla, ALLEGRO_KEY_DOWN) && movimiento_activado && juego_iniciado) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) + 1 <= 100) juego.direccion = abajo;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_UP) && movimiento_activado && juego_iniciado) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) - 1 >= 0) juego.direccion = arriba;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_LEFT) && movimiento_activado && juego_iniciado) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 2 + 5)  != '*' && ((juego.coord_x - 66) / 5) - 1 + 5 >= 0) juego.direccion = izquierda;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_RIGHT) && movimiento_activado && juego_iniciado) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 2 + 5) != '*' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100) juego.direccion = derecha;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_ESCAPE) && juego_iniciado) {
                movimiento_activado = false;
                al_rest(1);
                al_play_sample(morir, 0.5, 1, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            if (!movimiento_activado) contador++;

            switch (juego.direccion) {
            case abajo:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) + 1 <= 100 && movimiento_activado && juego_iniciado) {
                    juego.coord_y += velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case arriba:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) - 1 >= 0 && movimiento_activado && juego_iniciado) {
                    juego.coord_y -= velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case izquierda:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 1 + 5) != '*' && ((juego.coord_x - 66) / 5) - 1  + 5 >= 0 && movimiento_activado && juego_iniciado) {
                    juego.coord_x -= velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case derecha:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 1 + 5) != '*' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100 && movimiento_activado && juego_iniciado) {
                    juego.coord_x += velocidad;
                    juego.animacion++;
                }
                else{
                    juego.animacion = 0;
                }
                break;
            }

            if (inicializacion == 0) {
                al_play_sample(intro, 0.5, 1, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            inicializacion++;
            if (inicializacion == 70) {
                juego_iniciado = true;
            }
        }

        if ((juego.animacion > juego.direccion + 3 || juego.direccion != juego.direccion_previa) && movimiento_activado && juego_iniciado) juego.animacion = juego.direccion;

        if (*(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) == 'o') {
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = ' ';
            al_play_sample(comer, 0.2, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 10;
        }
        sprintf_s(juego.cifra, "%i", jugador.puntaje);
        sprintf_s(juego.impresion_nivel, "%i", jugador.nivel);

        if (movimiento_activado) {
            al_draw_bitmap(pacman[juego.animacion], juego.coord_x, juego.coord_y, NULL);
        }
        else {
            al_draw_bitmap(pacman[contador], juego.coord_x, juego.coord_y - 2, NULL);
            al_rest(0.1);
        }

        (jugador.mapa_actual == 1) ? al_draw_bitmap(mapa1, 75, 80, NULL) : al_draw_bitmap(mapa2, 75, 80, NULL);
        al_draw_bitmap(negro, 6, 79, NULL); 
        al_draw_bitmap(negro, 525, 79, NULL); 
        al_draw_text(formato, al_map_rgb(255, 163, 1), 100, 20, NULL, "NOMBRE");
        al_draw_text(formato, al_map_rgb(255, 163, 1), 230, 20, NULL, "PUNTAJE");
        al_draw_text(formato, al_map_rgb(255, 163, 1), 360, 20, NULL, "NIVEL");
        al_draw_text(formato, al_map_rgb(255, 163, 1), 460, 20, NULL, "VIDAS");
        
        switch (strlen(jugador.nombre)) {
        case 1: al_draw_text(formato, al_map_rgb(255, 255, 39), 132, 45, NULL, jugador.nombre); break;
        case 2: al_draw_text(formato, al_map_rgb(255, 255, 39), 124, 45, NULL, jugador.nombre); break;
        case 3: al_draw_text(formato, al_map_rgb(255, 255, 39), 119, 45, NULL, jugador.nombre); break;
        case 4: al_draw_text(formato, al_map_rgb(255, 255, 39), 113, 45, NULL, jugador.nombre); break;
        case 5: al_draw_text(formato, al_map_rgb(255, 255, 39), 107, 45, NULL, jugador.nombre); break;
        case 6: al_draw_text(formato, al_map_rgb(255, 255, 39), 100, 45, NULL, jugador.nombre); break;
        }

        switch (strlen(juego.cifra)) {
        case 1: al_draw_text(formato, al_map_rgb(255, 255, 39), 267, 45, NULL, juego.cifra); break;
        case 2: al_draw_text(formato, al_map_rgb(255, 255, 39), 262, 45, NULL, juego.cifra); break;
        case 3: al_draw_text(formato, al_map_rgb(255, 255, 39), 257, 45, NULL, juego.cifra); break;
        case 4: al_draw_text(formato, al_map_rgb(255, 255, 39), 252, 45, NULL, juego.cifra); break;
        case 5: al_draw_text(formato, al_map_rgb(255, 255, 39), 245, 45, NULL, juego.cifra); break;
        case 6: al_draw_text(formato, al_map_rgb(255, 255, 39), 238, 45, NULL, juego.cifra); break;
        case 7: al_draw_text(formato, al_map_rgb(255, 255, 39), 231, 45, NULL, juego.cifra); break;
        case 8: al_draw_text(formato, al_map_rgb(255, 255, 39), 228, 45, NULL, juego.cifra); break;
        }

        switch (strlen(juego.impresion_nivel)) {
        case 1: al_draw_text(formato, al_map_rgb(255, 255, 39), 380, 45, NULL, juego.impresion_nivel); break;
        case 2: al_draw_text(formato, al_map_rgb(255, 255, 39), 375, 45, NULL, juego.impresion_nivel); break;
        }

        switch (jugador.vidas) {
            case 1:
                al_draw_bitmap(icono, 445, 40, NULL);
                break;
            case 2:
                al_draw_bitmap(icono, 445, 40, NULL);
                al_draw_bitmap(icono, 470, 40, NULL);
                break;
            case 3:
                al_draw_bitmap(icono, 445, 40, NULL);
                al_draw_bitmap(icono, 470, 40, NULL);
                al_draw_bitmap(icono, 495, 40, NULL);
                break;
        }

        for (int i = 0; i < 101; i++) {
            for (int j = 0; j < 91; j++) {
                if (*(*(mapa + i) + j) == 'o') {     
                    indicador = true;
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(pac_dot, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(pac_dot, ((j - 5) * 5) + 72, (i * 5) + 76, NULL);
                }
            }
        }

        if (((juego.coord_y - 70) / 5) - 1 == 45 && (((juego.coord_x - 66) / 5) + 5) == 1) {
            juego.coord_x = (96 - 5) * 5 + 75;
        }
        if (((juego.coord_y - 70) / 5) - 1 == 45 && (((juego.coord_x - 66) / 5) + 5) == 98) {
            juego.coord_x = (2 - 5) * 5 + 75;
        }
        if (((juego.coord_y - 70) / 5) - 1 == 11 && (((juego.coord_x - 66) / 5) + 5) == 1) {
            juego.coord_x = (96 - 5) * 5 + 75;
        }
        if (((juego.coord_y - 70) / 5) - 1 == 11 && (((juego.coord_x - 66) / 5) + 5) == 98) {
            juego.coord_x = (2 - 5) * 5 + 75;
        }
        if (((juego.coord_y - 70) / 5) - 1 == 82 && (((juego.coord_x - 66) / 5) + 5) == 1) {
            juego.coord_x = (96 - 5) * 5 + 75;
        }
        if (((juego.coord_y - 70) / 5) - 1 == 82 && (((juego.coord_x - 66) / 5) + 5) == 98) {
            juego.coord_x = (2 - 5) * 5 + 75;
        }

        if (!juego_iniciado) {
            if(jugador.mapa_actual == 1 ) al_draw_bitmap(marcador, 253, 255, NULL);
            else al_draw_bitmap(marcador, 253, 270, NULL);
        }

        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));

        if (contador == 27) {
            jugador.vidas--;
            return true;
        }

        if (!indicador) juego.continuar_nivel = false;
    }
    return false;
}

char* generar_codigo() {
    char * codigo = new char [7];

    for (int i = 0; i < 6; i++) {
        codigo[i] = letras_aleatorias();
    }
    return codigo;
}

char letras_aleatorias() {
    int x = rand() % (26) + 1;

    switch (x) {
    case 1: return 'A'; break; case 2: return 'B'; break; case 3: return 'C'; break;
    case 4: return 'D'; break; case 5: return 'E'; break; case 6: return 'F'; break;
    case 7: return 'G'; break; case 8: return 'H'; break; case 9: return 'I'; break;
    case 10: return 'J'; break; case 11: return 'K'; break; case 12: return 'L'; break;
    case 13: return 'M'; break; case 14: return 'N'; break; case 15: return 'O'; break;
    case 16: return 'P'; break; case 17: return 'Q'; break; case 18: return 'R'; break;
    case 19: return 'S'; break; case 20: return 'T'; break; case 21: return 'U'; break;
    case 22: return 'V'; break; case 23: return 'W'; break; case 24: return 'X'; break;
    case 25: return 'Y'; break; case 26: return 'Z'; break; default: return '1';
    }
}

void continuar() {
    fopen("registro.dat", "rb");
}

void checar_records(ALLEGRO_DISPLAY* pantalla, FILE* registro) {
    Datos lista;
    bool regresar_menu = false, reanudar;
    int contador = 0, mayor, intercambio;
    char cifra[9], auxiliar[7];

    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_BITMAP* records = al_load_bitmap("img/tabla.png");
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();

    al_draw_bitmap(menu, 13, 30, NULL);
    al_draw_bitmap(records, 163, 240, NULL);
    al_draw_text(formato, al_map_rgb(255, 255, 39), 208, 550, NULL, "PRESIONA ENTER");
    al_draw_text(formato, al_map_rgb(255, 255, 39), 212, 570, NULL, "PARA REGRESAR");

    registro = fopen("registro.dat", "rb");

    fseek(registro, 0, SEEK_END);
    contador = ftell(registro) / sizeof(Datos);

    int* valores = new int[contador];
    char** nombres = new char* [contador];
    for (int i = 0; i < contador; i++) {
        *(nombres + i) = new char[7];
    }
    contador = 0;

    if (ftell(registro) > 0) {
        fseek(registro, 0, SEEK_SET);
        fread(&lista, sizeof(Datos), 1, registro);
        while (!feof(registro)) {
            *(valores + contador) = lista.puntaje;
            strcpy_s(nombres[contador], 7, lista.nombre);
            printf("%s\n", lista.nombre);
            fread(&lista, sizeof(Datos), 1, registro); 
            printf("%s\n", lista.nombre);
            if (!feof(registro)) contador++;
        }
        rewind(registro);

        if (contador > 0) {
            for (int i = 0; i <= contador; i++) {
                mayor = i;
                for (int j = i + 1; j <= contador; j++) {
                    if (valores[j] > valores[mayor]) {
                        mayor = j;
                    }
                }
                intercambio = valores[i];
                valores[i] = valores[mayor];
                valores[mayor] = intercambio;
                strcpy_s(auxiliar, 7, nombres[i]);
                strcpy_s(nombres[i], 7, nombres[mayor]);
                strcpy_s(nombres[mayor], 7, auxiliar);
            }
        }

        if (contador >= 8) {
            for (int i = 0; i < 8; i++) {
                sprintf_s(cifra, "%i", valores[i]);
                al_draw_text(formato, al_map_rgb(255, 255, 39), 200, 325 + (i * 27), NULL, nombres[i]);
                al_draw_text(formato, al_map_rgb(255, 255, 39), 340, 325 + (i * 27), NULL, cifra);
            }
        }
        else{
            for (int i = 0; i <= contador; i++) {
                sprintf_s(cifra, "%i", valores[i]);
                al_draw_text(formato, al_map_rgb(255, 255, 39), 200, 325 + (i * 27), NULL, nombres[i]);
                al_draw_text(formato, al_map_rgb(255, 255, 39), 340, 325 + (i * 27), NULL, cifra);
            }
        }   
    }

    al_flip_display();
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_display_event_source(pantalla));

    while (!regresar_menu) {
        ALLEGRO_EVENT evento;

        al_wait_for_event(fila_evento, &evento);
        switch (evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                regresar_menu = true;
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);
            }
            break;
        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
            reanudar = false;
            while (!reanudar) {
                ALLEGRO_EVENT evento2;
                al_wait_for_event(fila_evento, &evento2);

                if (evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_bitmap(records, 163, 240, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 208, 550, NULL, "PRESIONA ENTER");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 212, 570, NULL, "PARA REGRESAR");
                    
                    if (contador >= 8) {
                        for (int i = 0; i < 8; i++) {
                            sprintf_s(cifra, "%i", valores[i]);
                            al_draw_text(formato, al_map_rgb(255, 255, 39), 200, 325 + (i * 27), NULL, nombres[i]);
                            al_draw_text(formato, al_map_rgb(255, 255, 39), 340, 325 + (i * 27), NULL, cifra);
                        }
                    }
                    else {
                        for (int i = 0; i <= contador; i++) {
                            sprintf_s(cifra, "%i", valores[i]);
                            al_draw_text(formato, al_map_rgb(255, 255, 39), 200, 325 + (i * 27), NULL, nombres[i]);
                            al_draw_text(formato, al_map_rgb(255, 255, 39), 340, 325 + (i * 27), NULL, cifra);
                        }
                    }
                    al_flip_display();
                    reanudar = true;
                }
            }
            break;
        }
    }
    rewind(registro);
    fclose(registro);
}