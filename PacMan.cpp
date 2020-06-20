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
    char codigo[10];
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
void comenzar_juego(ALLEGRO_DISPLAY*, ALLEGRO_SAMPLE*, ALLEGRO_SAMPLE_ID);
Datos preguntar_nombre(ALLEGRO_DISPLAY*);
char convertir_letra(ALLEGRO_EVENT);
Mapas llenar_mapa1();
Mapas llenar_mapa2();
void movimiento_pacman(char* mapa[], Datos& jugador, Movimiento& juego, int velocidad);
void continuar();
void checar_records();

int main(int argc, char** argv) {
    srand(time(NULL));
    inicializar_graficos();
    bool salir = false, empezar = false, sonido = true, reanudar;
    int x = 0, y = 0;
    
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
                comenzar_juego(pantalla, OST, id);
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

void comenzar_juego(ALLEGRO_DISPLAY* pantalla, ALLEGRO_SAMPLE* OST, ALLEGRO_SAMPLE_ID id) {
    Datos jugador;
    Movimiento juego;
    Mapas tablero;
    char* mapa[111];
    bool finalizado = false;
    const int velocidad = 5;

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
            juego.coord_y = 352;
            tablero = llenar_mapa2(); 
            break;
        }
        juego.coord_x = 287;
        juego.animacion = 0;
        juego.continuar_nivel = true;

        for (int i = 0; i < 111; i++) {
            *(mapa + i) = tablero.mapa[i];;
        }

        movimiento_pacman(mapa, jugador, juego, velocidad);
    }
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
    {"     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ***o    o    o   o   o   o    o    o   o********o   o    o    o   o   o   o   o    o******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     ***o*********o****************o**************************o****************o********o******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     ***o*********o****************o**************************o****************o********o******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     *** ********* **************** ************************** **************** ******** ******     ",
     "     ***o    o    o   o   o   o    o    o   o    o   o   o    o    o   o   o   o   o    o******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     ***o***********************************o********o**********************************o******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     *** *********************************** ******** ********************************** ******     ",
     "     ***o*********o     o******************* ******** ******************o     o*********o******     ",
     "     ******************* *********o    o    o    o   o    o    o******** **********************     ",
     "     ******************* ********* **** ****************** **** ******** **********************     ",
     "     ******************* ********* **** ****************** **** ******** **********************     ",
     "     ******************* ********* **** ****************** **** ******** **********************     ",
     "     *******************o*********o     ******************     o********o**********************     ",
     "     ******************* ********* **** ****************** **** ******** **********************     ",
     "                     *** ********* **** ****************** **** ******** *****                      ",
     "                     *** ********* **** ****************** **** ******** *****                      ",
     "                     *** ********* **** ****************** **** ******** *****                      ",
     "                     ***o    o    o                            o    o   o*****                      ",
     "                     *** ********* ************** ************* ******** *****                      ",
     "                     *** ********* ************** ************* ******** *****                      ",
     "                     *** ********* ************** ************* ******** *****                      ",
     "                     *** ********* ************** ************* ******** *****                      ",
     "                     ***o********* ************** ************* ********o*****                      ",
     "                     *** ********* **************************** ******** *****                      ",
     "************************ ********* **************************** ******** ***************************",
     "************************ ********* ****                   ***** ******** ***************************",
     "************************ ********* ****                   ***** ******** ***************************",
     "*       o   o   o   o   o          ****                   *****         o   o   o   o   o          *",
     "************************ ********* ****                   ***** ******** ***************************",
     "************************ ********* ****                   ***** ******** ***************************",
     "     ******************* ********* ****                   ***** ******** **********************     ",
     "     ******************* ********* ****                   ***** ******** **********************     ",
     "     *******************o********* ****                   ***** ********o**********************     ",
     "     ******************* ********* ****                   ***** ******** **********************     ",
     "     ******************* ********* **************************** ******** **********************     ",
     "                     *** ********* **************************** ******** *****                      ",
     "                     *** *********                              ******** *****                      ",
     "                     ***o********* **************************** ********o*****                      ",
     "                     *** ********* **************************** ******** *****                      ",
     "                     *** ********* **************************** ******** *****                      ",
     "                     *** ********* **************************** ******** *****                      ",
     "                     *** ********* **************************** ********o*****                      ",
     "                     ***o********* **************************** ******** *****                      ",
     "     ******************* ********* **************************** ******** **********************     ",
     "     ******************* ********* **************************** ******** **********************     ",
     "     ******************* ********* **************************** ******** **********************     ",
     "     ***o    o    o     o    o    o    o    o********o    o    o   o    o     o    o    o******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     ***o***************o*******************o********o******************o***************o******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     ***o     o*********o    o    o    o    o   o    o    o    o   o    o*********o     o******     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     *********o*********o***********************o***********************o*********o************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ********* ********* *********************** *********************** ********* ************     ",
     "     ***o     o*********o*********o    o    o   o    o   o    o*********o*********o     o******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     ***o***************o*******************o********o******************o***************o******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     *** *************** ******************* ******** ****************** *************** ******     ",
     "     ***o     o    o    o    o    o    o    o   o    o     o   o   o    o    o    o     o******     ",
     "     ******************************************************************************************     ",
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


void movimiento_pacman(char *mapa[], Datos &jugador, Movimiento &juego, int velocidad) {

    ALLEGRO_BITMAP* pacman[27];
    ALLEGRO_BITMAP* mapa1 = al_load_bitmap("img/mapa1B.png");
    ALLEGRO_BITMAP* mapa2 = al_load_bitmap("img/mapa2.png");
    ALLEGRO_BITMAP* pac_dot = al_load_bitmap("img/sprites/map/pill.png");
    ALLEGRO_BITMAP* negro = al_load_bitmap("img/negro.png");
    ALLEGRO_BITMAP* icono = al_load_bitmap("img/sprites/ui/life.png"); 
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_TIMER* temporizador = al_create_timer(1.0 / 15);
    ALLEGRO_KEYBOARD_STATE estado_tecla;
    ALLEGRO_SAMPLE* comer = al_load_sample("sounds/waka.mp3");
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);

    al_register_event_source(fila_evento, al_get_timer_event_source(temporizador));
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_start_timer(temporizador);

    for (int i = 0; i < 27; i++) {
        strcpy_s(juego.nombre_archivo, 30, "img/pacman/pacman");
        sprintf_s(juego.numero, 3, "%i", i + 1);
        strcat_s(juego.nombre_archivo, 30, juego.numero);
        strcat_s(juego.nombre_archivo, 30, ".png");
        pacman[i] = al_load_bitmap(juego.nombre_archivo);
        al_convert_mask_to_alpha(pacman[i], al_map_rgb(0, 0, 0));
    }

    while (juego.continuar_nivel) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        if(evento.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&estado_tecla);
            juego.direccion_previa = juego.direccion;

            if (al_key_down(&estado_tecla, ALLEGRO_KEY_DOWN)) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) + 1 <= 100) juego.direccion = abajo;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_UP)) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) - 1 >= 0) juego.direccion = arriba;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_LEFT)) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 2 + 5)  != '*' && ((juego.coord_x - 66) / 5) - 1 + 5 >= 0) juego.direccion = izquierda;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_RIGHT)) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 2 + 5) != '*' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100) juego.direccion = derecha;
            }
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_ESCAPE)) {
                juego.continuar_nivel = false;
            }

            switch (juego.direccion) {
            case abajo:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) + 1 <= 100) {
                    juego.coord_y += velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case arriba:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && ((juego.coord_y - 70) / 5) - 1 >= 0) {
                    juego.coord_y -= velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case izquierda:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 1 + 5) != '*' && ((juego.coord_x - 66) / 5) - 1  + 5 >= 0) {
                    juego.coord_x -= velocidad;
                    juego.animacion++;
                }
                else {
                    juego.animacion = 0;
                }
                break;
            case derecha:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 1 + 5) != '*' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100) {
                    juego.coord_x += velocidad;
                    juego.animacion++;
                }
                else{
                    juego.animacion = 0;
                }
                break;
            }
        }

        if (juego.animacion > juego.direccion + 3 || juego.direccion != juego.direccion_previa) juego.animacion = juego.direccion;

        if (*(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) == 'o') {
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = ' ';
            al_play_sample(comer, 0.2, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 10;
        }
        sprintf_s(juego.cifra, "%i", jugador.puntaje);
        sprintf_s(juego.impresion_nivel, "%i", jugador.nivel);

        al_draw_bitmap(pacman[juego.animacion], juego.coord_x, juego.coord_y, NULL);
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
                al_draw_bitmap(icono, 490, 45, NULL);
                break;
            case 2:
                al_draw_bitmap(icono, 490, 45, NULL);
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
                    al_draw_bitmap(pac_dot, ((j - 5) * 5) + 72, (i * 5) + 76, NULL);
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

        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));
    }
}

void continuar() {

}

void checar_records() {

}