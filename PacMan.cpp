#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct Datos {
    char nombre[9];
    int puntaje;
};

void inicializar_graficos();
void comenzar_juego(ALLEGRO_DISPLAY*);
Datos preguntar_nombre(ALLEGRO_DISPLAY*);
char convertir_letra(ALLEGRO_EVENT);
bool verificar_cadena(char[]);
void continuar();
void checar_records();
void movimiento_pacman();

int main(int argc, char** argv) {
    srand(time(NULL));
    inicializar_graficos();
    bool salir = false, empezar = false, sonido = true, reanudar;
    int x, y;
    
    ALLEGRO_DISPLAY* pantalla = al_create_display(600, 600);
    ALLEGRO_FONT* formato = al_load_ttf_font("04B_30__.ttf", 17, NULL);
    ALLEGRO_SAMPLE* opcion = al_load_sample("sounds/selection.mp3");
    ALLEGRO_SAMPLE* click = al_load_sample("sounds/click.mp3");
    ALLEGRO_SAMPLE* OST = al_load_sample("sounds/Mega Man 3 (NES) Music Title Theme.mp3");
    ALLEGRO_SAMPLE_ID id;
    ALLEGRO_BITMAP* pantalla_inicio = al_load_bitmap("img/pantInicioa.png");
    ALLEGRO_BITMAP* boton_A = al_load_bitmap("img/presiona.png");
    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_BITMAP* opcion1 = al_load_bitmap("img/iniciar.png");
    ALLEGRO_BITMAP* opcion2 = al_load_bitmap("img/continuar.png");
    ALLEGRO_BITMAP* opcion3 = al_load_bitmap("img/records.png");
    ALLEGRO_BITMAP* opcion4 = al_load_bitmap("img/salir.png");
    //ALLEGRO_BITMAP* mapa1 = al_load_bitmap("img/Mapa1A.png");
    //ALLEGRO_BITMAP* mapa2 = al_load_bitmap("img/Mapa1B.png");
    //ALLEGRO_BITMAP* mapa3 = al_load_bitmap("img/Mapa2.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();

    al_reserve_samples(3);
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_mouse_event_source());
    al_register_event_source(fila_evento, al_get_display_event_source(pantalla));

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
                comenzar_juego(pantalla);
            }
            if (x > 212 && x < 388 && y > 340 && y < 372) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                continuar();
            }
            if (x > 225 && x < 373 && y > 400 && y < 432) {
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                checar_records();
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

void comenzar_juego(ALLEGRO_DISPLAY* pantalla) {
    Datos jugador;

    jugador = preguntar_nombre(pantalla);
    movimiento_pacman();
}

Datos preguntar_nombre(ALLEGRO_DISPLAY* pantalla) {
    Datos jugador;
    bool finalizado = false, reanudar;
    char cadena[9] = "", auxiliar;
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
    al_draw_text(formato, al_map_rgb(255, 255, 39), 237, 320, NULL, cadena);
    al_flip_display();
    
    while (!finalizado) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch(evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if(evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if(strlen(cadena) == 0){
                    al_show_native_message_box(pantalla, "Advertencia", "Error de formato", "Nombre mal introducido", NULL, ALLEGRO_MESSAGEBOX_WARN);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 237, 320, NULL, cadena);
                }
                else {
                    al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    finalizado = true;
                }
            }
            else {
                al_play_sample(tecla, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                auxiliar = convertir_letra(evento);
                fflush(stdin);
                if (auxiliar != '+' && auxiliar != '-' && contador < 8) {
                    cadena[contador] = auxiliar;

                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 237, 320, NULL, cadena);
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
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 237, 320, NULL, cadena);
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
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 237, 320, NULL, cadena);
                    al_flip_display();
                    reanudar = true;
                }
            }
            break;
        }
    }

    jugador.puntaje = 0;
    strcpy_s(jugador.nombre, 9, cadena);

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
    case ALLEGRO_KEY_8: letra = '8'; break; case ALLEGRO_KEY_9: letra = '9'; break; case ALLEGRO_KEY_BACKSPACE: letra = '-'; break;
    default: letra = '+'; break;
    }
    return letra;
}

void continuar() {

}

void checar_records() {

}

void movimiento_pacman() {
    enum { abajo = 0, arriba = 4, izquierda = 8, derecha = 12 };
    char nombre_archivo[30], numero[3];
    int coord_x = 300, coord_y = 300, velocidad = 5, animacion = 0, direccion = abajo, direccion_previa = abajo;
    bool terminado = false;

    ALLEGRO_BITMAP* pacman[24];
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_TIMER* temporizador = al_create_timer(1.0 / 15);

    al_register_event_source(fila_evento, al_get_timer_event_source(temporizador));
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_start_timer(temporizador);

    for (int i = 0; i < 24; i++) {
        strcpy_s(nombre_archivo, 30, "img/pacman/pacman");
        sprintf_s(numero, 3, "%i", i + 1);
        strcat_s(nombre_archivo, 30, numero);
        strcat_s(nombre_archivo, 30, ".png");
        pacman[i] = al_load_bitmap(nombre_archivo);
        al_convert_mask_to_alpha(pacman[i], al_map_rgb(0, 0, 0));
    }

    while (!terminado) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch (evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            direccion_previa = direccion;
            switch (evento.keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:
                direccion = abajo;
                break;
            case ALLEGRO_KEY_UP:
                direccion = arriba;
                break;
            case ALLEGRO_KEY_LEFT:
                direccion = izquierda;
                break;
            case ALLEGRO_KEY_RIGHT:
                direccion = derecha;
                break;
            case ALLEGRO_KEY_ESCAPE:
                terminado = true;
                break;
            }
            break;
        case ALLEGRO_EVENT_TIMER:
            direccion_previa = direccion;
            switch (direccion) {
            case abajo:
                if (coord_y < 570) coord_y += velocidad;
                break;
            case arriba:
                if (coord_y > 0) coord_y -= velocidad;
                break;
            case izquierda:
                if (coord_x > 0) coord_x -= velocidad;
                break;
            case derecha:
                if (coord_x < 570) coord_x += velocidad;
                break;
            }
            animacion++;
            break;
        }

        if (animacion > direccion + 3 || direccion != direccion_previa) animacion = direccion;
        al_draw_bitmap(pacman[animacion], coord_x, coord_y, NULL);
        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));
    }
}
