/*
Proyecto Final - PacMan.

Integrantes de equipo:
    - Erik Alejandro Gómez Martínez.
    - Israel Alejandro Mora González.
    - Ángel Gabriel Galindo López.
    - José Emmanuel Rodríguez López.

Fecha de entrega: 06 de julio de 2020.

Descripción general: Recreación en lenguaje C del clásico arcade PacMan. Existen varias funciones elementales para el funcionamiento
del proyecto:
    - Comenzar_juego: Se encarga de la efectuación de las partidas, así como del almacenado de contraseñas y puntajes.
    - Récords: Genera un ranking de las ocho puntuaciones más altas que se hayan registrado.
    - Continuar_juego: Permite reanudar una partida a través de una contraseña previamente obtenida.

El juego consta de seis niveles distintos; los impares se presentarán con el mapa principal, mientras que el resto con una variante.

Para avanzar de nivel es necesario que todos los Pac-Dots sean consumidos; las frutas solo funcionan como bonificadores.

Solo se puede almacenar un récord y contraseña por persona, evitando así la sobresaturación de los archivos binarios;
el sistema reemplazará automáticamente los respectivos datos según el progreso y puntaje de un jugador específico.

Juego programado en el IDE Visual Studio 2019, utilizando la versión 5.2.6 de Allegro.
*/

//- Librerías.
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

//- Enumeraciones.
enum lista { abajo = 0, arriba = 4, izquierda = 8, derecha = 12 }; //Movimiento de PacMan.
enum movimiento_fantasmas { abajo_s = 0, izquierda_s = 2, derecha_s = 4, arriba_s = 6 }; //Movimiento de fantasmas.

//- Estructuras.
struct Datos {
    char nombre[7];
    char codigo[7];
    int puntaje, nivel, vidas, mapa_actual;
}; //Datos del jugador.

struct Movimiento {
    int coord_x, coord_y, animacion; 
    bool continuar_nivel;
    char nombre_archivo[30],  numero[3], cifra[9], impresion_nivel[3];
    lista direccion = abajo, direccion_previa = abajo;
}; //Variables para la efectuación de las partidas.

struct Fantasma {
    int coord_x, coord_y, animacion = 0, estado = 1, puerta_cruzada = 0, retraso = 0, invisibilidad = 0;
    bool giro = false, movimiento = true;
    movimiento_fantasmas direccion = abajo_s, direccion_previa = abajo_s;
}; //Variables para el comportamiento de los fantasmas.

//- Uniones.
union Mapas {
    char mapa[101][101];
}; //Retorno de los mapas.

union Codigo{
    char cadena[7];
}; //Retorno de la generación aleatoria de contraseñas.


//- Prototipos de funciones.

//Inicialización de Allegro.
void inicializar_graficos(); 

//Solicitar nombre del usuario, llevar a cabo las partidas y registrar datos en respectivos archivos binarios.
void comenzar_juego(ALLEGRO_DISPLAY*, FILE*, Datos);

//Solicitar el nombre y/o contraseña del usuario dependiendo del proceso a realizar.
Datos preguntar_nombre(ALLEGRO_DISPLAY*, int);

//Impresión de instrucciones específicas para la solicitud de nombres.
void imprimir_texto();

//Convertir los eventos de teclado a caracteres. Permite el correcto ingreso de nombres y contraseñas.
char convertir_letra(ALLEGRO_EVENT);

//LLenado y retorno del primer mapa.
Mapas llenar_mapa1();

//LLenado y retorno del segundo mapa.
Mapas llenar_mapa2();

//Motor funcional de las partidas del juego. Controla los movimientos de PacMan y fantasmas; además, delimita temporizadores.
bool movimiento_pacman(char* mapa[], Datos& jugador, Movimiento& juego, int velocidad);

//Verificación del estado actual de los fantasmas. El contacto con ellos puede ocasionar la pérdida de una vida o bonificación.
bool verificar_muerte(char* mapa[], Movimiento juego, Fantasma enemigo, bool movimiento_activado);

//Gestiona el comportamiento de los fantasmas.
void mover_fantasma(char* mapa[], Movimiento juego, Datos jugador, int velocidad, Fantasma& enemigo, bool juego_iniciado, bool movimiento_activado, bool juego_pausado);

//Delimita las zonas de teletransporte para los fantasmas.
void definir_teletransporte_fantasmas(char* mapa[], Fantasma& enemigo);

//Se retornan direcciones aleatorias para los fantasmas.
movimiento_fantasmas conversion();

//Se genera un código para la reanudación de las partidas.
Codigo generar_codigo();

//Aleatoriza caracteres alfanuméricos para generar aleatoriamente cada contraseña.
char letras_aleatorias();

//Permite que los usuarios puedan reanudar sus partidas.
Datos continuar(ALLEGRO_DISPLAY*);

//Muestra el ranking de las ocho puntuaciones más altas registradas.
void checar_records(ALLEGRO_DISPLAY*, FILE*);

//- Función main.
int main(int argc, char** argv) {
    srand(time(NULL));
    inicializar_graficos();
    Datos jugador_nuevo, jugador_comenzar;
    bool salir = false, empezar = false, sonido = true, reanudar, regresar;
    int x = 0, y = 0;

    FILE* registro = fopen("registro.dat", "rb");
    if (!registro) registro = fopen("registro.dat", "ab");
    fclose(registro); //Se verifica la apertura del archivo para el almacenado de récords.
    
    ALLEGRO_DISPLAY* pantalla = al_create_display(600, 600);
    ALLEGRO_SAMPLE* opcion = al_load_sample("sounds/selection.mp3");
    ALLEGRO_SAMPLE* click = al_load_sample("sounds/click.mp3");
    ALLEGRO_SAMPLE* OST = al_load_sample("sounds/Mega Man 3 (NES) Music Title Theme.mp3");
    ALLEGRO_SAMPLE* menus = al_load_sample("sounds/Undertale OST 002   Start Menu.mp3");
    ALLEGRO_SAMPLE_ID id, id2;
    ALLEGRO_BITMAP* pantalla_inicio = al_load_bitmap("img/pantInicioa.png");
    ALLEGRO_BITMAP* icono = al_load_bitmap("img/sprites/fruits/cherry.png");
    ALLEGRO_BITMAP* boton_A = al_load_bitmap("img/presiona.png");
    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_BITMAP* opcion1 = al_load_bitmap("img/iniciar.png");
    ALLEGRO_BITMAP* opcion2 = al_load_bitmap("img/continuar.png");
    ALLEGRO_BITMAP* opcion3 = al_load_bitmap("img/records.png");
    ALLEGRO_BITMAP* opcion4 = al_load_bitmap("img/salir.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 18, NULL);
    ALLEGRO_FONT* formato2 = al_load_ttf_font("fonts/04B_30__.ttf", 15, NULL);

    al_reserve_samples(6);
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_mouse_event_source());
    al_register_event_source(fila_evento, al_get_display_event_source(pantalla)); //Inicializadores para el registro de eventos.
    al_set_window_title(pantalla, "PacMan"); //Cambiar el nombre de la ventana a "PacMan".
    al_set_display_icon(pantalla, icono); //Cambiar el ícono de la ventana.

    al_draw_bitmap(pantalla_inicio, 13, 30, NULL);
    al_draw_bitmap(boton_A, 232, 380, NULL);
    al_flip_display();

    al_play_sample(OST, 0.4, 0, 1.0, ALLEGRO_PLAYMODE_LOOP, &id); //Música de pantalla de título.

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
            } //Si el usuario presiona la tecla "A" se avanzará hacia el menú principal.
            break;
        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT: //El contenido se vuelve a mostrar en caso de cerrar la ventana.
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
    } //Finalización de pantalla de título.

    al_stop_sample(&id);

    al_play_sample(menus, 0.3, 0.5, 1, ALLEGRO_PLAYMODE_LOOP, &id2); //Cambio de música.
    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(menu, 13, 30, NULL);
        al_draw_bitmap(opcion1, 232, 280, NULL);
        al_draw_bitmap(opcion2, 212, 340, NULL);
        al_draw_bitmap(opcion3, 225, 400, NULL);
        al_draw_bitmap(opcion4, 249, 460, NULL);
        al_flip_display(); //Apertura de menú prinicipal.

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
            if (x > 232 && x < 360 && y > 280 && y < 312) { //Selección de "comenzar".
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);

                jugador_nuevo = preguntar_nombre(pantalla, 1);
                al_stop_sample(&id2);
                jugador_nuevo.nivel = 1; jugador_nuevo.vidas = 3;
                jugador_nuevo.mapa_actual = 1; //Se cargan los datos de inicio para el mapa y conteo del puntaje.

                comenzar_juego(pantalla, registro, jugador_nuevo); //Se comienza el juego.
                al_play_sample(menus, 0.3, 0.5, 1, ALLEGRO_PLAYMODE_LOOP, &id2);
            }
            if (x > 212 && x < 388 && y > 340 && y < 372) { //Selección de "continuar".
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);

                jugador_comenzar = continuar(pantalla); //Se extraen los datos de un usuario para verificar el cargado de una partida.

                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);

                if (jugador_comenzar.mapa_actual != -1) {
                    al_stop_sample(&id2);
                    comenzar_juego(pantalla, registro, jugador_comenzar);
                    al_play_sample(menus, 0.3, 0.5, 1, ALLEGRO_PLAYMODE_LOOP, &id2);
                } //El usuario y contraseña fueron validados.
                else {
                    regresar = false;
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 172, 280, NULL, "CODIGO O USUARIO");
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 220, 305, NULL, "INCORRECTO");
                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 187, 370, NULL, "PRESIONE LA TECLA");
                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 285, 390, NULL, "(Z)");
                    al_flip_display();

                    while (!regresar) {
                        ALLEGRO_EVENT evento3;
                        al_wait_for_event(fila_evento, &evento3);

                        switch (evento3.type) {
                        case ALLEGRO_EVENT_KEY_DOWN:
                            if (evento3.keyboard.keycode == ALLEGRO_KEY_Z) {
                                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                                al_rest(0.3);
                                al_clear_to_color(al_map_rgb(0, 0, 0));
                                al_flip_display();
                                al_rest(0.3);
                                regresar = true;
                            }
                            break;
                        case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                            reanudar = false;
                            while (!reanudar) {
                                ALLEGRO_EVENT evento4;
                                al_wait_for_event(fila_evento, &evento4);

                                if (evento4.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                                    reanudar = true;
                                    al_draw_bitmap(menu, 13, 30, NULL);
                                    al_draw_text(formato, al_map_rgb(255, 255, 39), 172, 280, NULL, "CODIGO O USUARIO");
                                    al_draw_text(formato, al_map_rgb(255, 255, 39), 220, 305, NULL, "INCORRECTO");
                                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 187, 370, NULL, "PRESIONE LA TECLA");
                                    al_draw_text(formato2, al_map_rgb(255, 255, 39), 285, 390, NULL, "(Z)");
                                    al_flip_display();
                                }
                            }
                            break;
                        }
                    }
                }//Las entradas fueron inválidas.
            }
            if (x > 225 && x < 373 && y > 400 && y < 432) { //Selección de "récords".
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                al_rest(0.3);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_flip_display();
                al_rest(0.3);

                checar_records(pantalla, registro); //Generación del ranking.
            }
            if (x > 249 && x < 343 && y > 460 && y < 492) { //Selección de "salir".
                al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                salir = true;
                al_stop_sample(&id2);
            }
            break;
        }
    }
    return 0;
}

//- Implementación de funciones.
 
//Inicialización de Allegro.
void inicializar_graficos() {
    al_init(); 
    al_install_audio(); //Instalar audio.
    al_install_keyboard(); //Instalar teclado.
    al_install_mouse(); //Instalar mouse.
    al_init_font_addon(); //Fuentes.
    al_init_ttf_addon(); //Fuentes.
    al_init_image_addon(); //Cargado de imágenes.
    al_init_acodec_addon(); //Audio.
}

//Solicitar nombre del usuario, llevar a cabo las partidas y registrar datos en respectivos archivos binarios.
void comenzar_juego(ALLEGRO_DISPLAY* pantalla, FILE* registro, Datos jugador) {
    Datos auxiliar;
    Movimiento juego;
    Mapas tablero;
    Codigo clave;
    char* mapa[111];
    bool finalizado = false, nombre_encontrado = false, vida_perdida, reanudar;
    const int velocidad = 5;
    int puntuacion_previa, continuar_juego, continuar = false;

    FILE* codigos = fopen("contraseñas.dat", "rb+");
    if (!codigos) {
        codigos = fopen("contraseñas.dat", "ab");
        fclose(codigos);
        codigos = fopen("contraseñas.dat", "rb+");
    } //Se verifica la apetura del archivo para el registro de contraseñas.

    ALLEGRO_SAMPLE* fin_nivel = al_load_sample("sounds/07 - Round Clear.mp3");
    ALLEGRO_SAMPLE* click = al_load_sample("sounds/click.mp3");
    ALLEGRO_BITMAP* menu = al_load_bitmap("img/t2.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_FONT* formato1 = al_load_ttf_font("fonts/04B_30__.ttf", 18, NULL);
    ALLEGRO_FONT* formato2 = al_load_ttf_font("fonts/04B_30__.ttf", 15, NULL);
    ALLEGRO_FONT* formato3 = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);
    al_register_event_source(fila_evento, al_get_keyboard_event_source());

    registro = fopen("registro.dat", "rb+"); //Apertura del archivo para el registro de récords.

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
        } //Selección del mapa actual.
        juego.coord_x = 287;
        juego.animacion = 0;
        juego.continuar_nivel = true;
        juego.direccion = abajo;
        juego.direccion_previa = abajo;
        vida_perdida = false;
        continuar_juego = 0;

        for (int i = 0; i < 111; i++) {
            *(mapa + i) = tablero.mapa[i];
        }

        puntuacion_previa = jugador.puntaje;
        vida_perdida = movimiento_pacman(mapa, jugador, juego, velocidad); //Se verifica su hubo pérdida de vidas o no.

        if (!vida_perdida) {
            jugador.nivel++;
            al_play_sample(fin_nivel, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

            al_rest(2.5);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);

            if (jugador.nivel != 7) {

                al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "NIVEL FINALIZADO");
                al_draw_bitmap(menu, 13, 30, NULL);
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
                        if (evento.keyboard.keycode == ALLEGRO_KEY_A) { //Se da inicio al siguiente nivel.
                            al_play_sample(click, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                            continuar_juego = 1;
                        }
                        if (evento.keyboard.keycode == ALLEGRO_KEY_S) { //La partida actual finaliza por selección del usuario.
                            al_play_sample(click, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                            continuar_juego = 2;
                        }
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
            }    
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);
        } //El nivel actual fue completado exitosamente.
        else {
            jugador.puntaje = puntuacion_previa;
            al_rest(1.5);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            al_rest(1.0);
        } //El jugador perdió una vida.
                     
        //Selección del mapa actual según el nivel contador.
        if (jugador.vidas > 0) (jugador.nivel % 2 == 0) ? jugador.mapa_actual = 2 : jugador.mapa_actual = 1; 

        //El juego termina si el usuario pierde todas sus vidas o finaliza el nivel 6.
        if (jugador.vidas <= 0 || jugador.nivel >= 7 || continuar_juego == 2) finalizado = true;
    }

    
    if (jugador.nivel != 7 && jugador.nivel != 1) {
        clave = generar_codigo();
        strcpy_s(jugador.codigo, 7, clave.cadena);
    } //Se genera una contraseña de reanudación si el usuario no terminó los seis niveles.

    if (jugador.vidas <= 0) jugador.vidas = 1; //El jugador comenzará con una vida si las perdió todas.
    
    al_draw_bitmap(menu, 13, 30, NULL);
    al_draw_text(formato1, al_map_rgb(255, 255, 39), 176, 280, NULL, "JUEGO FINALIZADO");
    al_draw_text(formato2, al_map_rgb(255, 255, 39), 187, 350, NULL, "PRESIONE LA TECLA");
    al_draw_text(formato2, al_map_rgb(255, 255, 39), 285, 370, NULL, "(Q)");
    if (jugador.nivel != 7 && jugador.nivel != 1) {
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 213, 420, NULL, "LA CLAVE PARA");
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 170, 440, NULL, "REANUDAR LA PARTIDA");
        al_draw_text(formato2, al_map_rgb(255, 255, 39), 288, 460, NULL, "ES");
        al_draw_text(formato2, al_map_rgb(255, 163, 1), 260, 490, NULL, jugador.codigo);
    } //Se muestra la contraseña generada al usuario.
    al_flip_display();

    while (!continuar) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch (evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if (evento.keyboard.keycode == ALLEGRO_KEY_Q) { //Se regresará al menú principal si se presiona la tecla "Q".
                al_play_sample(click, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
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
                    if (jugador.nivel != 7 && jugador.nivel != 1) {
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
    } //Finalización de la partida.

    al_rest(0.3);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_rest(0.3);

    //Registro de las contraseñas y récords
    if (jugador.nivel != 7 && jugador.nivel != 1) {
        fseek(codigos, 0, SEEK_END);
        if (ftell(codigos) == 0) {
            fwrite(&jugador, sizeof(Datos), 1, codigos);
        } //El archivo se llenará de forma directa si está vacío.
        else {
            fseek(codigos, 0, SEEK_SET);
            fread(&auxiliar, sizeof(Datos), 1, codigos);
            while (!feof(codigos)) {
                if (strcmp(auxiliar.nombre, jugador.nombre) == 0) {
                    nombre_encontrado = true;
                    fseek(codigos, ftell(codigos) - sizeof(Datos), SEEK_SET);
                    fwrite(&jugador, sizeof(Datos), 1, codigos);
                    break;
                } //Si el nombre a registrar ya se encuentra en el archivo se reemplazarán los datos.
                fread(&auxiliar, sizeof(Datos), 1, codigos);
            }

            if (!nombre_encontrado) {
                fseek(codigos, 0, SEEK_END);
                fwrite(&jugador, sizeof(Datos), 1, codigos);
            } //El nombre se escribirá por primera vez si no se encontró en el archivo ya llenado.
        }
    } //Registro de contraseñas.
    rewind(codigos);
    fclose(codigos);
    nombre_encontrado = false;

    fseek(registro, 0, SEEK_END);
    if (ftell(registro) == 0) {
        if (jugador.puntaje > 0) fwrite(&jugador, sizeof(Datos), 1, registro); //Se registra un récord mayor a 0.
    }
    else {
        fseek(registro, 0, SEEK_SET);
        fread(&auxiliar, sizeof(Datos), 1, registro);
        while (!feof(registro)) {
            if (strcmp(auxiliar.nombre, jugador.nombre) == 0) {
                nombre_encontrado = true;
                if (jugador.puntaje > auxiliar.puntaje && jugador.puntaje > 0) {
                    fseek(registro, ftell(registro) - sizeof(Datos), SEEK_SET);
                    fwrite(&jugador, sizeof(Datos), 1, registro);
                } //Si el puntaje actual supera al registrado, entonces los datos se reemplazarán.
                break;
            } //El usuario a registrar fue encontrado en el archivo.
            fread(&auxiliar, sizeof(Datos), 1, registro);
        }

        if (!nombre_encontrado) {
            fseek(registro, 0, SEEK_END);
            if (jugador.puntaje > 0) fwrite(&jugador, sizeof(Datos), 1, registro);
        }
    } //Registro de los récords.
    rewind(registro);
    fclose(registro);
}

//Solicitar el nombre y/o contraseña del usuario dependiendo del proceso a realizar.
Datos preguntar_nombre(ALLEGRO_DISPLAY* pantalla, int texto) {
    Datos jugador;
    bool finalizado = false, reanudar;
    char cadena[7] = "", auxiliar, cadena1[7] = "";
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
    imprimir_texto(); //Se muestra la interfaz para la solicitud del nombre.
    al_flip_display();
    
    while (!finalizado) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        switch(evento.type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            if(evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if(strlen(cadena) == 0){
                    al_show_native_message_box(pantalla, "Advertencia", "Error de formato", "Texto mal introducido", NULL, ALLEGRO_MESSAGEBOX_WARN);
                } //Si no se introdujo un nombre, entonces el programa lo advertirá.
                else {
                    al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    finalizado = true;
                } //El nombre fue validado.
            } //Envío del nombre.
            else {
                auxiliar = convertir_letra(evento); //Conversión de eventos de teclado a caracteres.
                if(auxiliar != '+' && strlen(cadena) < 6) al_play_sample(tecla, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                fflush(stdin);
                if (auxiliar != '+' && auxiliar != '-' && contador < 6) {
                    cadena[contador] = auxiliar;

                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_bitmap(menu, 13, 30, NULL);
                    al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU NOMBRE");
                    imprimir_texto();
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
                    imprimir_texto();
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
            } //Ingreso del nombre.
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
                    imprimir_texto();
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

    //Se solicitará un código en la respectiva opción.
    if (texto == 2) {
        finalizado = false;
        contador = 0;

        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(menu, 13, 30, NULL);
        al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU CODIGO");
        imprimir_texto();
        al_flip_display();

        while (!finalizado) {
            ALLEGRO_EVENT evento;
            al_wait_for_event(fila_evento, &evento);

            switch (evento.type) {
            case ALLEGRO_EVENT_KEY_DOWN:
                if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    if (strlen(cadena1) == 0) {
                        al_show_native_message_box(pantalla, "Advertencia", "Error de formato", "Texto mal introducido", NULL, ALLEGRO_MESSAGEBOX_WARN);
                    } //Código no introducido.
                    else {
                        al_play_sample(click, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        finalizado = true;
                    } //Envío del código.
                } 
                else {
                    auxiliar = convertir_letra(evento);
                    if (auxiliar != '+' && strlen(cadena1) < 6) al_play_sample(tecla, 0.6, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    fflush(stdin);
                    if (auxiliar != '+' && auxiliar != '-' && contador < 6) {
                        cadena1[contador] = auxiliar;

                        al_clear_to_color(al_map_rgb(0, 0, 0));
                        al_draw_bitmap(menu, 13, 30, NULL);
                        al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU CODIGO");
                        imprimir_texto();
                        switch (strlen(cadena1)) {
                        case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena1); break;
                        case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena1); break;
                        case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena1); break;
                        case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena1); break;
                        case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena1); break;
                        case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena1); break;
                        }
                        al_flip_display();

                        contador++;
                    }
                    if (auxiliar == '-' && contador >= 0) {
                        if (contador > 0) contador--;
                        cadena1[contador] = NULL;

                        al_clear_to_color(al_map_rgb(0, 0, 0));
                        al_draw_bitmap(menu, 13, 30, NULL);
                        al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU CODIGO");
                        imprimir_texto();
                        switch (strlen(cadena1)) {
                        case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena1); break;
                        case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena1); break;
                        case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena1); break;
                        case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena1); break;
                        case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena1); break;
                        case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena1); break;
                        }
                        al_flip_display();
                    }
                    fflush(stdin);
                } //Introducción del código.
                al_flip_display();
                break;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                reanudar = false;
                while (!reanudar) {
                    ALLEGRO_EVENT evento2;
                    al_wait_for_event(fila_evento, &evento2);

                    if (evento2.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
                        al_draw_bitmap(menu, 13, 30, NULL);
                        al_draw_text(formato, al_map_rgb(255, 255, 39), 160, 280, NULL, "INGRESE SU CODIGO");
                        imprimir_texto();
                        switch (strlen(cadena1)) {
                        case 1: al_draw_text(formato, al_map_rgb(255, 163, 1), 299, 340, NULL, cadena1); break;
                        case 2: al_draw_text(formato, al_map_rgb(255, 163, 1), 290, 340, NULL, cadena1); break;
                        case 3: al_draw_text(formato, al_map_rgb(255, 163, 1), 281, 340, NULL, cadena1); break;
                        case 4: al_draw_text(formato, al_map_rgb(255, 163, 1), 272, 340, NULL, cadena1); break;
                        case 5: al_draw_text(formato, al_map_rgb(255, 163, 1), 263, 340, NULL, cadena1); break;
                        case 6: al_draw_text(formato, al_map_rgb(255, 163, 1), 254, 340, NULL, cadena1); break;
                        }
                        al_flip_display();
                        reanudar = true;
                    }
                }
                break;
            }
        }
    }

    jugador.puntaje = 0;
    strcpy_s(jugador.nombre, 7, cadena);
    strcpy_s(jugador.codigo, 7, cadena1);
    //Se preparan los datos para su retorno a través de una estructura.

    return jugador;
}

//Impresión de instrucciones específicas para la solicitud de nombres.
void imprimir_texto() {
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 18, NULL);

    al_draw_text(formato, al_map_rgb(255, 255, 39), 183, 500, NULL, "PRESIONE ENTER");
    al_draw_text(formato, al_map_rgb(255, 255, 39), 204, 530, NULL, "AL FINALIZAR");
}

//Convertir los eventos de teclado a caracteres. Permite el correcto ingreso de nombres y contraseñas.
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
    return letra; //Se retornará un caracrer correspondiente a la tecla presionada por el usuario.
}

//LLenado y retorno del primer mapa.
Mapas llenar_mapa1() {
    Mapas tablero;
    char mapa[101][101] =
    {"     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     *******************************************************************************************    ",
     "     ***Q    o    q   o   o   o    q    o   q********q   o    q    o   o   o   q    o    Q******    ",
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
     "     ***q    o    q   o   o   o    q    o   q    o   q   o    q    o   o   o   q    o    q******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     ***o***********************************o********o***********************************o******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     *** *********************************** ******** *********************************** ******    ",
     "     ***q*********q     q******************* ******** *******************q     q*********q******    ",
     "     ******************* *********q    o    q    o   q    o    q********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "     *******************o*********o     ******************     o*********o**********************    ",
     "     ******************* ********* **** ****************** **** ********* **********************    ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     *** ********* **** ****************** **** ********* *****                     ",
     "                     ***q    o    q              M             q    o    q*****                     ",
     "                     *** ********* **************X************* ********* *****                     ",
     "                     *** ********* **************X************* ********* *****                     ",
     "                     *** ********* **************X************* ********* *****                     ",
     "                     *** ********* **************X************* ********* *****                     ",
     "                     ***o********* **************X************* *********o*****                     ",
     "                     *** ********* **************X************* ********* *****                     ",
     "************************ ********* **************X************* ********* **************************",
     "************************ ********* **************X************* ********* **************************",
     "************************ ********* **************X************* ********* **************************",
     "*       o   o   o   o   q         p********p     p     p*******p         q   o   o   o   o         *",
     "************************ ********* **************************** ********* **************************",
     "************************ ********* **************************** ********* **************************",
     "     ******************* ********* **************************** ********* **********************    ",
     "     ******************* ********* **************************** ********* **********************    ",
     "     *******************o********* **************************** *********o**********************    ",
     "     ******************* ********* **************************** ********* **********************    ",
     "     ******************* ********* **************************** ********* **********************    ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** *********p                            p********* *****                     ",
     "                     ***o********* **************************** *********o*****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** ********* *****                     ",
     "                     *** ********* **************************** *********o*****                     ",
     "                     ***o********* **************************** ********* *****                     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ***q    o    o     q    o    q    o    q********q    o    q   o     q     o    o    q*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o***************o*******************o********o*******************o***************q*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***q     q*********q    o    o    o    q   q    q    o    o   o     q*********q     q*****     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     *********o*********o***********************o************************o*********o***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ********* ********* *********************** ************************ ********* ***********     ",
     "     ***q     q*********o*********q    o    q   q    q   o    q**********o*********q     q*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***o***************o*******************o********o*******************o***************o*****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     *** *************** ******************* ******** ******************* *************** *****     ",
     "     ***Q     o    o    q    o    o    o    q   o    q     o   o   o     q    o    o     Q*****     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
    }; //Inicialización del primer mapa.
    /*
    Simbología:
        o - Pac-Dot.
        p - Cambio de dirección en fantasmas.
        q - Pac-Dot y cambio de dirección para fantasmas.
        X - Puerta.
        M - Cierre de puerta.
        Q - Potenciador.
        * - Colisión.
    */

    for (int i = 0; i < 101; i++) {
        strcpy_s(*(tablero.mapa + i), 101, mapa[i]);
    }
    return tablero;
}

//LLenado y retorno del segundo mapa.
Mapas llenar_mapa2() {
    Mapas tablero;
    char mapa[101][101] =
    {"     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ***Q    o    o    q   o    o   q   o   o*********q   o   q   o   o   q    o    o    Q*****     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ******************o************o*************************o***********o********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "*********************** ************ ************************* *********** *************************",
     "*********************** ************ ************************* *********** *************************",
     "*      o   o   o   o   q************o*************************o***********q   o   o   o   o        *",
     "*********************** ************ ************************* *********** *************************",
     "*********************** ************ ************************* *********** *************************",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ******************o************o*************************o***********o********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ****************** ************ ************************* *********** ********************     ",
     "     ***q    o    o    q   o    o   q   o   q    o   q    o   q   o   o   q    o    o    q*****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     ***o***********************************o********o***********************************o*****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********************************** ******** *********************************** *****     ",
     "     *** *********q     q******************* ******** *******************q     q********* *****     ",
     "     ***q*************** *********q    o    q    o   q    o    q********* ***************q*****     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     *******************o*********o     ******************     o*********o*********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ******************* ********* **** ****************** **** ********* *********************     ",
     "     ***q   o   o   o   q    o    q             M              q    o    q   o   o   o   q*****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     ***o***************o********* *************X************** *********o***************o*****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     *** *************** ********* *************X************** ********* *************** *****     ",
     "     ***q     q*********q         p******* p    p     p********p         q*********q     q*****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     ***o***************o********* **************************** *********o***************o*****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     *** *************** ********* **************************** ********* *************** *****     ",
     "     ***q   o   o   o   q*********p                            p*********q   o   o   o   q*****     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     *******************o********* **************************** *********o*********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ******************* ********* **************************** ********* *********************     ",
     "     ***p**********p    q    o    q    o    o*********q    o   q    o    q     p*********p*****     ",
     "     *** ********** **** ******************* ********* ****************** ***** ********* *****     ",
     "     *** ********** **** ******************* ********* ****************** ***** ********* *****     ",
     "     *** ********** **** ******************* ********* ****************** ***** ********* *****     ",
     "     *** ********** **** ******************* ********* ****************** ***** ********* *****     ",
     "     *** ********** **** *******************o*********o****************** ***** ********* *****     ",
     "     *** ********** **** ******************* ********* ****************** ***** ********* *****     ",
     "     ***p          p    p******************* ********* ******************p     p         p*****     ",
     "     ******************* ******************* ********* ****************** *********************     ",
     "     ******************* ******************* ********* ****************** *********************     ",
     "     *******************q    o    o    o    o*********q    o   o    o    q*********************     ",
     "     ******************* ************************************************ *********************     ",
     "     ******************* ************************************************ *********************     ",
     "************************ ************************************************ **************************",
     "************************ ************************************************ **************************",
     "*                       q************************************************q                         *",
     "************************ ************************************************ **************************",
     "************************ ************************************************ **************************",
     "     ******************* ************************************************ *********************     ",
     "     ******************* *********Q    o    q********q    o    Q********* *********************     ",
     "     ******************* ******************* ******** ******************* *********************     ",
     "     ******************* ******************* ******** ******************* *********************     ",
     "     ***q     o    q    q******************* ******** *******************q     q    o    q*****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** *******************o********o******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     *** ********** **** ******************* ******** ******************* ***** ********* *****     ",
     "     ***q**********q    q    o    o    o    q    o   q    o    o   o     q     q*********q*****     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
     "     ******************************************************************************************     ",
    }; //Inicialización de segundo mapa.
    /*Simbología:
        o - Pac - Dot.
        p - Cambio de dirección en fantasmas.
        q - Pac - Dot y cambio de dirección para fantasmas.
        X - Puerta.
        M - Cierre de puerta.
        Q - Potenciador.
        * - Colisión.
    */

    for (int i = 0; i < 101; i++) {
        strcpy_s(*(tablero.mapa + i), 101, mapa[i]);
    }
    return tablero;
}

//Motor funcional de las partidas del juego. Controla los movimientos de PacMan y fantasmas; además, delimita temporizadores.
bool movimiento_pacman(char *mapa[], Datos &jugador, Movimiento &juego, int velocidad) {
    ALLEGRO_BITMAP* pacman[28];
    ALLEGRO_BITMAP* blinky[8];
    ALLEGRO_BITMAP* clyde[8];
    ALLEGRO_BITMAP* pinky[8];
    ALLEGRO_BITMAP* inky[8];
    ALLEGRO_BITMAP* fantasma_afectado[2];
    ALLEGRO_BITMAP* fantasma_comido;
    ALLEGRO_BITMAP* fantasma_recuperacion[2];
    ALLEGRO_BITMAP* mapa1 = al_load_bitmap("img/mapa1B.png");
    ALLEGRO_BITMAP* mapa2 = al_load_bitmap("img/mapa2.png");
    ALLEGRO_BITMAP* pac_dot = al_load_bitmap("img/sprites/map/pill.png");
    ALLEGRO_BITMAP* pastilla = al_load_bitmap("img/sprites/map/big-0.png");
    ALLEGRO_BITMAP* negro = al_load_bitmap("img/negro.png");
    ALLEGRO_BITMAP* icono = al_load_bitmap("img/sprites/ui/life.png");
    ALLEGRO_BITMAP* marcador = al_load_bitmap("img/pacman/ready.png");
    ALLEGRO_BITMAP* naranja = al_load_bitmap("img/sprites/fruits/orange.png"); 
    ALLEGRO_BITMAP* campana = al_load_bitmap("img/sprites/fruits/bell.png");
    ALLEGRO_BITMAP* llave = al_load_bitmap("img/sprites/fruits/key.png");
    ALLEGRO_BITMAP* insignia = al_load_bitmap("img/sprites/fruits/boss.png");
    ALLEGRO_BITMAP* manzana = al_load_bitmap("img/sprites/fruits/apple.png");
    ALLEGRO_BITMAP* melon = al_load_bitmap("img/sprites/fruits/melon.png");
    ALLEGRO_BITMAP* fresa = al_load_bitmap("img/sprites/fruits/strawberry.png");
    ALLEGRO_BITMAP* cereza = al_load_bitmap("img/sprites/fruits/cherry.png");
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_TIMER* temporizador = al_create_timer(1.0 / 15);
    ALLEGRO_KEYBOARD_STATE estado_tecla;
    ALLEGRO_SAMPLE* comer = al_load_sample("sounds/waka.mp3");
    ALLEGRO_SAMPLE* morir = al_load_sample("sounds/pacmandying.mp3");
    ALLEGRO_SAMPLE* intro = al_load_sample("sounds/game_start.wav");
    ALLEGRO_SAMPLE* potenciador = al_load_sample("sounds/power_pellet.wav");
    ALLEGRO_SAMPLE* sonido_fantasmas = al_load_sample("sounds/siren_1.wav");
    ALLEGRO_SAMPLE* comer_fantasmas = al_load_sample("sounds/eat_ghost.wav");
    ALLEGRO_SAMPLE* comer_fruta = al_load_sample("sounds/eat_fruit.wav");
    ALLEGRO_FONT* formato = al_load_ttf_font("fonts/04B_30__.ttf", 13, NULL);
    ALLEGRO_FONT* formato2 = al_load_ttf_font("fonts/04B_30__.ttf", 10, NULL);
    ALLEGRO_SAMPLE_ID id1, id2;
    Fantasma blinky1, pinky1, inky1, clyde1;
    bool indicador, movimiento_activado = true, juego_iniciado = false, sonido_iniciado = false, vulnerabilidad = false, juego_pausado = false, sonido_activado = false;
    int contador = 15, inicializacion = 0, estado_fantasmas = 1, tiempo_vulnerabilidad = 0, puntaje_comer = 0, pausar_juego = 0;
    char acumulacion[8];

    blinky1.coord_x = 316; pinky1.coord_x = 253; clyde1.coord_x = 296; inky1.coord_x = 273;

    if (jugador.mapa_actual == 1) {
        blinky1.coord_y = 300;
        pinky1.coord_y = 300;
        inky1.coord_y = 300;
        clyde1.coord_y = 300;
    }
    else {
        blinky1.coord_y = 315;
        pinky1.coord_y = 315;
        inky1.coord_y = 315;
        clyde1.coord_y = 315;
    } //Inicialización de las coordenadas para los fantasmas.

    switch (jugador.nivel) {
    case 1:
        *(*(mapa + 26) + 18) = 'N';
        *(*(mapa + 26) + 79) = 'C';
        *(*(mapa + 84) + 34) = 'L';
        *(*(mapa + 84) + 62) = 'I';
        break;
    case 2:
        *(*(mapa + 30) + 8) = 'N';
        *(*(mapa + 30) + 89) = 'V';
        *(*(mapa + 68) + 8) = 'A';
        *(*(mapa + 68) + 89) = 'H';
        break;
    case 3:
        *(*(mapa + 4) + 8) = 'L';
        *(*(mapa + 4) + 89) = 'S';
        break;
    case 4: 
        *(*(mapa + 4) + 8) = 'H';
        *(*(mapa + 4) + 89) = 'A';
        break;
    case 5:
        *(*(mapa + 4) + 8) = 'L';
        *(*(mapa + 94) + 8) = 'C';
        *(*(mapa + 94) + 89) = 'I';
        break;
    case 6:
        *(*(mapa + 4) + 8) = 'A';
        *(*(mapa + 87) + 34) = 'V';
        *(*(mapa + 87) + 63) = 'S';
        break;
    } //Posicionamiento de frutas según el mapa.
    /*
    Simbología:
        A - Manzana.    L - Llave.
        C - Campana.    V - Melón.
        I - Insignia.   N - Naranja.
        H - Cereza.     S - Fresa.
    */

    al_reserve_samples(6);

    fantasma_afectado[0] = al_load_bitmap("img/sprites/ghosts/f-0.png");
    fantasma_afectado[1] = al_load_bitmap("img/sprites/ghosts/f-1.png");

    fantasma_recuperacion[0] = al_load_bitmap("img/sprites/ghosts/f-2.png");
    fantasma_recuperacion[1] = al_load_bitmap("img/sprites/ghosts/f-3.png");

    fantasma_comido = al_load_bitmap("img/sprites/eyes/d.png");

    al_register_event_source(fila_evento, al_get_timer_event_source(temporizador));
    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_start_timer(temporizador); //Inicio del temporizador del juego.

    for (int i = 0; i < 28; i++) {
        strcpy_s(juego.nombre_archivo, 30, "img/pacman/pacman");
        sprintf_s(juego.numero, 3, "%i", i + 1);
        strcat_s(juego.nombre_archivo, 30, juego.numero);
        strcat_s(juego.nombre_archivo, 30, ".png");
        pacman[i] = al_load_bitmap(juego.nombre_archivo);
        al_convert_mask_to_alpha(pacman[i], al_map_rgb(0, 0, 0));
    }

    for (int i = 0; i < 32; i++) {
        int j;
        if (i < 8) {
            strcpy_s(juego.nombre_archivo, 30, "img/blinky/blinky");
            sprintf_s(juego.numero, 3, "%i", i + 1);
            j = i;
        }
        if (i >= 8 && i < 16) {
            strcpy_s(juego.nombre_archivo, 30, "img/inky/inky");
            sprintf_s(juego.numero, 3, "%i", i - 7);
            j = i - 8;
        }
        if (i >= 16 && i < 24) {
            strcpy_s(juego.nombre_archivo, 30, "img/clyde/clyde");
            sprintf_s(juego.numero, 3, "%i", i - 15);
            j = i - 16;
        }
        if (i >= 24 && i < 32) {
            strcpy_s(juego.nombre_archivo, 30, "img/pinky/pinky");
            sprintf_s(juego.numero, 3, "%i", i - 23);
            j = i - 24;
        }
        strcat_s(juego.nombre_archivo, 30, juego.numero);
        strcat_s(juego.nombre_archivo, 30, ".png");

        if (i < 8) blinky[j] = al_load_bitmap(juego.nombre_archivo);
        if (i >= 8 && i < 16) inky[j] = al_load_bitmap(juego.nombre_archivo);
        if (i >= 16 && i < 24) clyde[j] = al_load_bitmap(juego.nombre_archivo);
        if (i >= 24 && i < 32) pinky[j] = al_load_bitmap(juego.nombre_archivo);
    } //Inicialización de bitmaps.

    while (juego.continuar_nivel) {
        indicador = false;
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_evento, &evento);

        if(evento.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&estado_tecla);
            juego.direccion_previa = juego.direccion;
            blinky1.direccion_previa = blinky1.direccion;
            pinky1.direccion_previa = pinky1.direccion;
            inky1.direccion_previa = inky1.direccion;
            clyde1.direccion_previa = clyde1.direccion;

            if (al_key_down(&estado_tecla, ALLEGRO_KEY_DOWN) && movimiento_activado && juego_iniciado && !juego_pausado) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && *(*(mapa + ((juego.coord_y - 70) / 5) + 2) + ((juego.coord_x - 66) / 5) + 5) != 'X' && ((juego.coord_y - 70) / 5) + 1 <= 100) juego.direccion = abajo;
            } //PacMan se direcciona hacia abajo si no hay ninguna pared.
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_UP) && movimiento_activado && juego_iniciado && !juego_pausado) {
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 2) + ((juego.coord_x - 66) / 5) + 5) != '*' && *(*(mapa + ((juego.coord_y - 70) / 5) - 2) + ((juego.coord_x - 66) / 5) + 5) != 'X' && ((juego.coord_y - 70) / 5) - 1 >= 0) juego.direccion = arriba;
            } //PacMan se direcciona hacia arriba si no hay ninguna pared.
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_LEFT) && movimiento_activado && juego_iniciado && !juego_pausado) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 2 + 5)  != '*' && *(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 2 + 5) != 'X' && ((juego.coord_x - 66) / 5) - 1 + 5 >= 0) juego.direccion = izquierda;
            } //PacMan se direcciona hacia la izquierda si no hay ninguna pared.
            if (al_key_down(&estado_tecla, ALLEGRO_KEY_RIGHT) && movimiento_activado && juego_iniciado && !juego_pausado) {
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 2 + 5) != '*' && *(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 2 + 5) != 'X' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100) juego.direccion = derecha;
            } //PacMan se direcciona hacia la derecha si no hay ninguna pared.
           
            if (verificar_muerte(mapa, juego, blinky1, movimiento_activado)) {
                if (blinky1.estado == 1) {
                    movimiento_activado = false;
                    switch (estado_fantasmas) {
                        case 1: al_stop_sample(&id1); break;
                        case 2: al_stop_sample(&id2); break;
                    }
                    al_rest(1);
                    al_play_sample(morir, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                } //Si Blinky se encontraba persiguiendo, entonces el jugador pierde una vida.
                if (blinky1.estado == 2) {
                    blinky1.estado = 3;

                    puntaje_comer += 200;
                    jugador.puntaje += puntaje_comer;

                    al_play_sample(comer_fantasmas, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                    juego_pausado = true;
                } //En caso contrario, el fantasma es comido por PacMan.
            } //Se verifica si hubo colisión con Blinky (fantasma rojo).
            if (verificar_muerte(mapa, juego, inky1, movimiento_activado)) {
                if (inky1.estado == 1) {
                    movimiento_activado = false;
                    switch (estado_fantasmas) {
                        case 1: al_stop_sample(&id1); break;
                        case 2: al_stop_sample(&id2); break;
                    }
                    al_rest(1);
                    al_play_sample(morir, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                } //Si Inky se encontraba persiguiendo, entonces el jugador pierde una vida.
                if (inky1.estado == 2) {
                    inky1.estado = 3;

                    puntaje_comer += 200;
                    jugador.puntaje += puntaje_comer;

                    al_play_sample(comer_fantasmas, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                    juego_pausado = true;
                } //En caso contrario, el fantasma es comido por PacMan.
            } //Se verifica si hubo colisión con Inky (fantasma azul).
            if (verificar_muerte(mapa, juego, pinky1, movimiento_activado)) {
                if (pinky1.estado == 1) {
                    movimiento_activado = false;
                    switch (estado_fantasmas) {
                    case 1: al_stop_sample(&id1); break;
                    case 2: al_stop_sample(&id2); break;
                    }
                    al_rest(1);
                    al_play_sample(morir, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                } //Si Pinky se encontraba persiguiendo, entonces el jugador pierde una vida.
                if (pinky1.estado == 2) {
                    pinky1.estado = 3;

                    puntaje_comer += 200;
                    jugador.puntaje += puntaje_comer;

                    al_play_sample(comer_fantasmas, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                    juego_pausado = true;
                } //En caso contrario, el fantasma es comido por PacMan.
            } //Se verifica si hubo colisión con Pinky (fantasma rosa).
            if (verificar_muerte(mapa, juego, clyde1, movimiento_activado)) {
                if (clyde1.estado == 1) {
                    movimiento_activado = false;
                    switch (estado_fantasmas) {
                    case 1: al_stop_sample(&id1); break;
                    case 2: al_stop_sample(&id2); break;
                    }
                    al_rest(1);
                    al_play_sample(morir, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                } //Si Clyde se encontraba persiguiendo, entonces el jugador pierde una vida.
                if (clyde1.estado == 2) {
                    clyde1.estado = 3;

                    puntaje_comer += 200;
                    jugador.puntaje += puntaje_comer;

                    al_play_sample(comer_fantasmas, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                    juego_pausado = true;
                } //En caso contrario, el fantasma es comido por PacMan.
            } //Se verifica si hubo colisión con Clyde (fantasma naranja).
          
            if (!movimiento_activado) contador++;
            if (juego_pausado) {
                pausar_juego++;
                sprintf_s(acumulacion, "%i", puntaje_comer);
                al_draw_text(formato2, al_map_rgb(255, 255, 255), juego.coord_x, juego.coord_y - 5, NULL, acumulacion);
            } //Se genera una breve pausa al comer un fantasma.

            if (pausar_juego == 15) {
                juego_pausado = false;
                pausar_juego = 0;
                
                if (blinky1.estado == 3 && blinky1.puerta_cruzada) {
                    if (jugador.mapa_actual == 1) {
                        blinky1.coord_y = 300;
                    }
                    else {
                        blinky1.coord_y = 315;
                    }
                    blinky1.coord_x = 316;
                }

                if (clyde1.estado == 3 && clyde1.puerta_cruzada) {
                    if (jugador.mapa_actual == 1) {
                        clyde1.coord_y = 300;
                    }
                    else {
                        clyde1.coord_y = 315;
                    }
                    clyde1.coord_x = 296;
                }

                if (pinky1.estado == 3 && pinky1.puerta_cruzada) {
                    if (jugador.mapa_actual == 1) {
                        pinky1.coord_y = 300;
                    }
                    else {
                        pinky1.coord_y = 315;
                    }
                    pinky1.coord_x = 253;
                }

                if (inky1.estado == 3 && inky1.puerta_cruzada) {
                    if (jugador.mapa_actual == 1) {
                        inky1.coord_y = 300;
                    }
                    else {
                        inky1.coord_y = 315;
                    }
                    inky1.coord_x = 273;
                }
            } // Los fantasmas consumidos regresan a su punto de origen.

            if (blinky1.estado == 3) {
                if (!juego_pausado) blinky1.invisibilidad++;
                blinky1.movimiento = false;
            } //Se activa la invulnerabilidad de Blinky tras ser comido.
            if (pinky1.estado == 3) {
                if (!juego_pausado) pinky1.invisibilidad++;
                pinky1.movimiento = false;
            } //Se activa la invulnerabilidad de Pinky tras ser comido.
            if (inky1.estado == 3) {
                if (!juego_pausado) inky1.invisibilidad++;
                inky1.movimiento = false;
            } //Se activa la invulnerabilidad de Inky tras ser comido.
            if (clyde1.estado == 3) {
                if (!juego_pausado) clyde1.invisibilidad++;
                clyde1.movimiento = false;
            } //Se activa la invulnerabilidad de Clyde tras ser comido.

            if (blinky1.invisibilidad == 70) {
                blinky1.estado = 1;
                blinky1.invisibilidad = 0;
                blinky1.movimiento = true;
            } //Se desactiva la invulnerabilidad de Blinky para volver a perseguir a PacMan.
            if (pinky1.invisibilidad == 70) {
                pinky1.estado = 1;
                pinky1.invisibilidad = 0;
                pinky1.movimiento = true;
            } //Se desactiva la invulnerabilidad de Pinky para volver a perseguir a PacMan.
            if (inky1.invisibilidad == 70) {
                inky1.estado = 1;
                inky1.invisibilidad = 0;
                inky1.movimiento = true;
            } //Se desactiva la invulnerabilidad de Inky para volver a perseguir a PacMan.
            if (clyde1.invisibilidad == 70) {
                clyde1.estado = 1;
                clyde1.invisibilidad = 0;
                clyde1.movimiento = true;
            } //Se desactiva la invulnerabilidad de Clyde para volver a perseguir a PacMan.
                
            mover_fantasma(mapa, juego, jugador, velocidad, blinky1, juego_iniciado, movimiento_activado, juego_pausado); //Movimiento de Blinky.
            mover_fantasma(mapa, juego, jugador, velocidad, pinky1, juego_iniciado, movimiento_activado, juego_pausado); //Movimiento de Pinky.
            mover_fantasma(mapa, juego, jugador, velocidad, inky1, juego_iniciado, movimiento_activado, juego_pausado); //Movimiento de Inky.
            mover_fantasma(mapa, juego, jugador, velocidad, clyde1, juego_iniciado, movimiento_activado, juego_pausado); //Movimiento de Clyde.

            switch (juego.direccion) {
            case abajo:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) + 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && *(*(mapa + ((juego.coord_y - 70) / 5) + 1) + ((juego.coord_x - 66) / 5) + 5) != 'X' && ((juego.coord_y - 70) / 5) + 1 <= 100 && movimiento_activado && juego_iniciado && !juego_pausado) {
                    juego.coord_y += velocidad;
                    juego.animacion++;
                } //PacMan se desplaza hacia abajo.
                else {
                    if (!juego_pausado) juego.animacion = 0;
                } //PacMan se detiene.
                break;
            case arriba:
                if (*(*(mapa + ((juego.coord_y - 70) / 5) - 1) + ((juego.coord_x - 66) / 5) + 5) != '*' && *(*(mapa + ((juego.coord_y - 70) / 5) - 1) + ((juego.coord_x - 66) / 5) + 5) != 'X' && ((juego.coord_y - 70) / 5) - 1 >= 0 && movimiento_activado && juego_iniciado && !juego_pausado) {
                    juego.coord_y -= velocidad;
                    juego.animacion++;
                } //PacMan se desplaza hacia arriba.
                else {
                    if (!juego_pausado) juego.animacion = 0;
                }
                break;
            case izquierda:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 1 + 5) != '*' && *(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) - 1 + 5) != 'X' && ((juego.coord_x - 66) / 5) - 1  + 5 >= 0 && movimiento_activado && juego_iniciado && !juego_pausado) {
                    juego.coord_x -= velocidad;
                    juego.animacion++;
                } //PacMan se desplaza hacia la izquierda.
                else {
                    if (!juego_pausado) juego.animacion = 0;
                }
                break;
            case derecha:
                if (*(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 1 + 5) != '*' && *(*(mapa + (juego.coord_y - 70) / 5) + ((juego.coord_x - 66) / 5) + 1 + 5) != 'X' && ((juego.coord_x - 66) / 5) + 1 + 5 <= 100 && movimiento_activado && juego_iniciado && !juego_pausado) {
                    juego.coord_x += velocidad;
                    juego.animacion++;
                } //PacMan se desplaza hacia la derecha.
                else{
                    if (!juego_pausado) juego.animacion = 0;
                }
                break;
            }

            if(!juego_iniciado){
                if (inicializacion == 0) {
                    al_play_sample(intro, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                } 
                inicializacion++;
                if (inicializacion == 70) {
                    juego_iniciado = true;
                }
            } //Se detienen los movimientos para el inicio del juego.
            
            if (estado_fantasmas == 2 && !juego_pausado) tiempo_vulnerabilidad++; //Se contabiliza el tiempo del potenciador.
        }

        if ((juego.animacion > juego.direccion + 3 || juego.direccion != juego.direccion_previa) && movimiento_activado && juego_iniciado) juego.animacion = juego.direccion; //Control de la animación de PacMan.

        switch (blinky1.estado) {
        case 1:
            if ((blinky1.animacion > blinky1.direccion + 1 || blinky1.direccion != blinky1.direccion_previa) && movimiento_activado && juego_iniciado) blinky1.animacion = blinky1.direccion;
            break;
        default:
            if (blinky1.animacion == 2 && movimiento_activado && juego_iniciado && blinky1.estado == 2) blinky1.animacion = 0;
            break;
        } //Control de las animaciones de Blinky.

        switch (pinky1.estado) {
        case 1:
            if ((pinky1.animacion > pinky1.direccion + 1 || pinky1.direccion != pinky1.direccion_previa) && movimiento_activado && juego_iniciado) pinky1.animacion = pinky1.direccion;
            break;
        default:
            if (pinky1.animacion == 2 && movimiento_activado && juego_iniciado && pinky1.estado == 2) pinky1.animacion = 0;
            break;
        } //Control de las animaciones de Pinky.

        switch (inky1.estado) {
        case 1:
            if ((inky1.animacion > inky1.direccion + 1 || inky1.direccion != inky1.direccion_previa) && movimiento_activado && juego_iniciado) inky1.animacion = inky1.direccion;
            break;
        default:
            if (inky1.animacion == 2 && movimiento_activado && juego_iniciado && inky1.estado == 2) inky1.animacion = 0;
            break;
        } //Control de las animaciones de Inky.

        switch (clyde1.estado) {
        case 1:
            if ((clyde1.animacion > clyde1.direccion + 1 || clyde1.direccion != clyde1.direccion_previa) && movimiento_activado && juego_iniciado) clyde1.animacion = clyde1.direccion;
            break;
        default:
            if (clyde1.animacion == 2 && movimiento_activado && juego_iniciado && clyde1.estado == 2) clyde1.animacion = 0;
            break;
        } //Control de las animaciones de Clyde.

        switch (*(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5)) {
        case 'o': 
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = ' ';
            al_play_sample(comer, 0.1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 10;
            break;
        case 'q':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer, 0.1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 10;
            break;
        case 'C':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 200;
            break;
        case 'L':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 500;
            break;
        case 'I':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 1000;
            break;
        case 'N':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 100;
            break;
        case 'V':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 100;
            break;
        case 'S':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 100;
            break;
        case 'H':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 100;
            break;
        case 'A':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            al_play_sample(comer_fruta, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 100;
            break;
        case 'Q':
            *(*(mapa + ((juego.coord_y - 70) / 5)) + ((juego.coord_x - 66) / 5) + 5) = 'p';
            tiempo_vulnerabilidad = 0;

            blinky1.animacion = 0; if (blinky1.estado != 3) blinky1.estado = 2;
            inky1.animacion = 0; if (inky1.estado != 3) inky1.estado = 2;
            pinky1.animacion = 0; if (pinky1.estado != 3) pinky1.estado = 2;
            clyde1.animacion = 0; if (clyde1.estado != 3) clyde1.estado = 2;

            estado_fantasmas = 2;
            if (!vulnerabilidad) sonido_iniciado = false;
            al_stop_sample(&id1);
            al_play_sample(comer, 0.1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            jugador.puntaje += 50;
            break;
        } //Eventos al comer potenciadores, Pac-Dots o frutas.

        sprintf_s(juego.cifra, "%i", jugador.puntaje);
        sprintf_s(juego.impresion_nivel, "%i", jugador.nivel);

        if (movimiento_activado) {
            al_draw_bitmap(pacman[juego.animacion], juego.coord_x, juego.coord_y, NULL);
        } //Impresión de PacMan.
        else {
            al_draw_bitmap(pacman[contador], juego.coord_x, juego.coord_y - 2, NULL);
            al_rest(0.1);
        } //Animación de muerte.

        //- Impresión de elementos en pantalla.
        (jugador.mapa_actual == 1) ? al_draw_bitmap(mapa1, 75, 80, NULL) : al_draw_bitmap(mapa2, 75, 80, NULL);
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
                switch (*(*(mapa + i) + j)) {
                case 'o':
                case 'q':
                    indicador = true;
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(pac_dot, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(pac_dot, ((j - 5) * 5) + 72, (i * 5) + 76, NULL);
                    break;
                case 'Q':
                    indicador = true;
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(pastilla, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(pastilla, ((j - 5) * 5) + 72, (i * 5) + 76, NULL);
                    break;
                case 'A':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(manzana, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(manzana, ((j - 5) * 5) + 70, (i * 5) + 71, NULL);
                    break;
                case 'C':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(campana, ((j - 5) * 5) + 67, (i * 5) + 73, NULL) : al_draw_bitmap(campana, ((j - 5) * 5) + 67, (i * 5) + 74, NULL);
                    break;
                case 'I':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(insignia, ((j - 5) * 5) + 67, (i * 5) + 69, NULL) : al_draw_bitmap(insignia, ((j - 5) * 5) + 66, (i * 5) + 69, NULL);
                    break;
                case 'H':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(cereza, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(cereza, ((j - 5) * 5) + 66, (i * 5) + 72, NULL);
                    break;
                case 'L':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(llave, ((j - 5) * 5) + 67, (i * 5) + 70, NULL) : al_draw_bitmap(llave, ((j - 5) * 5) + 66, (i * 5) + 70, NULL);
                    break;
                case 'V':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(melon, ((j - 5) * 5) + 73, (i * 5) + 76, NULL) : al_draw_bitmap(melon, ((j - 5) * 5) + 65, (i * 5) + 68, NULL);
                    break;
                case 'N':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(naranja, ((j - 5) * 5) + 67, (i * 5) + 73, NULL) : al_draw_bitmap(naranja, ((j - 5) * 5) + 69, (i * 5) + 69, NULL);
                    break;
                case 'S':
                    (jugador.mapa_actual == 1) ? al_draw_bitmap(fresa, ((j - 5) * 5) + 66, (i * 5) + 70, NULL) : al_draw_bitmap(fresa, ((j - 5) * 5) + 68, (i * 5) + 70, NULL);
                    break;
                }
            }
        }

        //- Eventos de teletransporte de PacMan.
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

        definir_teletransporte_fantasmas(mapa, blinky1);
        definir_teletransporte_fantasmas(mapa, pinky1);
        definir_teletransporte_fantasmas(mapa, inky1);
        definir_teletransporte_fantasmas(mapa, clyde1);

        if (!juego_iniciado) {
            if(jugador.mapa_actual == 1 ) al_draw_bitmap(marcador, 253, 255, NULL);
            else al_draw_bitmap(marcador, 253, 270, NULL);
        } //Impresión de la leyenda "Ready!".

        switch (estado_fantasmas) {
        case 1:
            if (!sonido_iniciado && juego_iniciado) {
                if(movimiento_activado) al_play_sample(sonido_fantasmas, 0.5, 1, 1, ALLEGRO_PLAYMODE_LOOP, &id1);
                sonido_iniciado = true;
            }
            if (pinky1.estado == 2) al_draw_bitmap(fantasma_afectado[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);
            else if (pinky1.estado == 3) al_draw_bitmap(fantasma_comido, pinky1.coord_x, pinky1.coord_y, NULL);
            else al_draw_bitmap(pinky[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);

            if (inky1.estado == 2) al_draw_bitmap(fantasma_afectado[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);
            else if (inky1.estado == 3) al_draw_bitmap(fantasma_comido, inky1.coord_x, inky1.coord_y, NULL);
            else al_draw_bitmap(inky[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);

            if (clyde1.estado == 2) al_draw_bitmap(fantasma_afectado[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);
            else if (clyde1.estado == 3) al_draw_bitmap(fantasma_comido, clyde1.coord_x, clyde1.coord_y, NULL);
            else al_draw_bitmap(clyde[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);

            if (blinky1.estado == 2) al_draw_bitmap(fantasma_afectado[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
            else if (blinky1.estado == 3) al_draw_bitmap(fantasma_comido, blinky1.coord_x, blinky1.coord_y, NULL);
            else al_draw_bitmap(blinky[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
            break; //Impresiones de las animaciones de los fantasmas.
        case 2:
            if (!sonido_iniciado) {
                if (movimiento_activado) al_play_sample(potenciador, 0.5, 0.5, 1, ALLEGRO_PLAYMODE_LOOP, &id2);
                sonido_iniciado = true;
                vulnerabilidad = true;
            } //Control del temporizador para el potenciador.
            if (blinky1.estado != 2 && pinky1.estado != 2 && inky1.estado != 2 && clyde1.estado != 2 && !juego_pausado) tiempo_vulnerabilidad = 200;
            if (tiempo_vulnerabilidad < 175) {
                if (pinky1.estado == 2) al_draw_bitmap(fantasma_afectado[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);
                else if (pinky1.estado == 3) al_draw_bitmap(fantasma_comido, pinky1.coord_x, pinky1.coord_y, NULL);
                else al_draw_bitmap(pinky[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);

                if (inky1.estado == 2) al_draw_bitmap(fantasma_afectado[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);
                else if (inky1.estado == 3) al_draw_bitmap(fantasma_comido, inky1.coord_x, inky1.coord_y, NULL);
                else al_draw_bitmap(inky[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);

                if (clyde1.estado == 2) al_draw_bitmap(fantasma_afectado[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);
                else if (clyde1.estado == 3) al_draw_bitmap(fantasma_comido, clyde1.coord_x, clyde1.coord_y, NULL);
                else al_draw_bitmap(clyde[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);

                if (blinky1.estado == 2) al_draw_bitmap(fantasma_afectado[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
                else if (blinky1.estado == 3) al_draw_bitmap(fantasma_comido, blinky1.coord_x, blinky1.coord_y, NULL);
                else al_draw_bitmap(blinky[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
            } //Impresión de fantasmas en color azul.
            else {
                if (tiempo_vulnerabilidad % 5 != 0) {
                    if (pinky1.estado == 2) al_draw_bitmap(fantasma_afectado[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);
                    else if (pinky1.estado == 3) al_draw_bitmap(fantasma_comido, pinky1.coord_x, pinky1.coord_y, NULL);
                    else al_draw_bitmap(pinky[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);

                    if (inky1.estado == 2) al_draw_bitmap(fantasma_afectado[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);
                    else if (inky1.estado == 3) al_draw_bitmap(fantasma_comido, inky1.coord_x, inky1.coord_y, NULL);
                    else al_draw_bitmap(inky[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);

                    if (clyde1.estado == 2) al_draw_bitmap(fantasma_afectado[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);
                    else if (clyde1.estado == 3) al_draw_bitmap(fantasma_comido, clyde1.coord_x, clyde1.coord_y, NULL);
                    else al_draw_bitmap(clyde[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);

                    if (blinky1.estado == 2) al_draw_bitmap(fantasma_afectado[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
                    else if (blinky1.estado == 3) al_draw_bitmap(fantasma_comido, blinky1.coord_x, blinky1.coord_y, NULL);
                    else al_draw_bitmap(blinky[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
                }
                else {
                    if (pinky1.estado == 2) al_draw_bitmap(fantasma_recuperacion[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);
                    else if (pinky1.estado == 3) al_draw_bitmap(fantasma_comido, pinky1.coord_x, pinky1.coord_y, NULL);
                    else al_draw_bitmap(pinky[pinky1.animacion], pinky1.coord_x, pinky1.coord_y, NULL);

                    if (inky1.estado == 2) al_draw_bitmap(fantasma_recuperacion[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);
                    else if (inky1.estado == 3) al_draw_bitmap(fantasma_comido, inky1.coord_x, inky1.coord_y, NULL);
                    else al_draw_bitmap(inky[inky1.animacion], inky1.coord_x, inky1.coord_y, NULL);

                    if (clyde1.estado == 2) al_draw_bitmap(fantasma_recuperacion[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);
                    else if (clyde1.estado == 3) al_draw_bitmap(fantasma_comido, clyde1.coord_x, clyde1.coord_y, NULL);
                    else al_draw_bitmap(clyde[clyde1.animacion], clyde1.coord_x, clyde1.coord_y, NULL);

                    if (blinky1.estado == 2) al_draw_bitmap(fantasma_recuperacion[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
                    else if (blinky1.estado == 3) al_draw_bitmap(fantasma_comido, blinky1.coord_x, blinky1.coord_y, NULL);
                    else al_draw_bitmap(blinky[blinky1.animacion], blinky1.coord_x, blinky1.coord_y, NULL);
                } // Impresión del parpadeo de fantasmas vulnerables.
            }

            if (tiempo_vulnerabilidad == 200) {
                al_stop_sample(&id2);
                puntaje_comer = 0;
                sonido_iniciado = false;

                tiempo_vulnerabilidad = 0;
                vulnerabilidad = false;

                estado_fantasmas = 1;
                if (blinky1.estado == 2) blinky1.estado = 1;
                if (inky1.estado == 2) inky1.estado = 1;
                if (pinky1.estado == 2) pinky1.estado = 1;
                if (clyde1.estado == 2) clyde1.estado = 1;
            } //Finalización de potenciador.
            break;
        }

        al_draw_bitmap(negro, 6, 79, NULL);
        al_draw_bitmap(negro, 525, 79, NULL);
        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));

        if (contador == 27) {
            jugador.vidas--;
            return true;
        } //Pérdida de vida.

        if (!indicador) juego.continuar_nivel = false; //Finalización correcta del nivel.
    }
    switch (estado_fantasmas) {
    case 1: al_stop_sample(&id1); break;
    case 2: al_stop_sample(&id2); break;
    }
    return false;
}

//Verificación del estado actual de los fantasmas. El contacto con ellos puede ocasionar la pérdida de una vida o bonificación.
bool verificar_muerte(char* mapa[], Movimiento juego, Fantasma enemigo, bool movimiento_activado) {
    if (((((juego.coord_x - 66) / 5) + 5) == (((enemigo.coord_x - 66) / 5) + 5) || (((juego.coord_x - 66) / 5) + 6) == (((enemigo.coord_x - 66) / 5) + 5) || (((juego.coord_x - 66) / 5) + 5) == (((enemigo.coord_x - 66) / 5) + 6)) && ((((juego.coord_y - 70) / 5)) == (((enemigo.coord_y - 70) / 5)) || (((juego.coord_y - 70) / 5) + 1) == ((enemigo.coord_y - 70) / 5) || ((juego.coord_y - 70) / 5) == (((enemigo.coord_y - 70) / 5) + 1)) && movimiento_activado) {
        return true;
    } //Fantasma colisionado.
    else {
        return false;
    } //El fantasma no colisíonó con PacMan.
}

//Gestiona el comportamiento de los fantasmas.
void mover_fantasma(char* mapa[], Movimiento juego, Datos jugador, int velocidad, Fantasma& enemigo, bool juego_iniciado, bool movimiento_activado, bool juego_pausado){
    if ((mapa[(enemigo.coord_y - 70) / 5][((enemigo.coord_x - 66) / 5) + 5] == 'p' || mapa[(enemigo.coord_y - 70) / 5][((enemigo.coord_x - 66) / 5) + 5] == 'Q' || mapa[(enemigo.coord_y - 70) / 5][((enemigo.coord_x - 66) / 5) + 5] == 'q') && !enemigo.giro && !juego_pausado) {
        enemigo.direccion = conversion();
        enemigo.giro = true;
    } //Verificar si el fantasma giró.
    if (enemigo.giro && !juego_pausado) enemigo.retraso++;
    if (enemigo.retraso == 6) {
        enemigo.retraso = 0;
        enemigo.giro = false;
    } //Retraso de giro.
    if (jugador.mapa_actual == 1) {
       if (((enemigo.coord_y - 70) / 5) == 46 && (((enemigo.coord_x - 66) / 5) + 5) == 49 && !juego_pausado) {
            enemigo.direccion = arriba_s;
       }
    }
    else {
        if (((enemigo.coord_y - 70) / 5) == 49 && (((enemigo.coord_x - 66) / 5) + 5) == 48 && !juego_pausado) {
            enemigo.direccion = arriba_s;
        }
    } //Movimientos predeterminados para que los fantasmas crucen la puerta.
   
    switch (enemigo.direccion) {
    case abajo_s:
        if (*(*(mapa + ((enemigo.coord_y - 70) / 5) + 1) + ((enemigo.coord_x - 66) / 5) + 5) != '*' && ((enemigo.coord_y - 70) / 5) + 1 <= 100 && movimiento_activado && juego_iniciado && !juego_pausado && enemigo.movimiento) {
            if (enemigo.puerta_cruzada == 0) {
                enemigo.coord_y += velocidad;
                enemigo.animacion++;
            }
            else {
                if (*(*(mapa + ((enemigo.coord_y - 70) / 5) + 1) + ((enemigo.coord_x - 66) / 5) + 5) != 'X') {
                    enemigo.coord_y += velocidad;
                    enemigo.animacion++;
                }
                else {
                    enemigo.direccion = conversion();
                }
            }
        } //El fantasma actual se direcciona hacia abajo.
        else {
            if (!juego_pausado && enemigo.movimiento) enemigo.direccion = conversion();
        } //El movimiento se aleatoriza.
        break;
    case arriba_s:
        if (*(*(mapa + ((enemigo.coord_y - 70) / 5) - 1) + ((enemigo.coord_x - 66) / 5) + 5) != '*' && ((enemigo.coord_y - 70) / 5) - 1 >= 0 && movimiento_activado && juego_iniciado && !juego_pausado && enemigo.movimiento) {
            enemigo.coord_y -= velocidad;
            enemigo.animacion++;
        } //El fantasma actual se direcciona hacia arriba.
        else {
            if (!juego_pausado && enemigo.movimiento) enemigo.direccion = conversion();
        } //El movimiento se aleatoriza.
        break;
    case derecha_s:
        if (*(*(mapa + (enemigo.coord_y - 70) / 5) + ((enemigo.coord_x - 66) / 5) + 1 + 5) != '*' && ((enemigo.coord_x - 66) / 5) + 1 + 5 <= 100 && movimiento_activado && juego_iniciado && !juego_pausado && enemigo.movimiento) {
            enemigo.coord_x += velocidad;
            enemigo.animacion++;
        } //El fantasma actual se direcciona hacia la derecha.
        else {
            if (!juego_pausado && enemigo.movimiento) enemigo.direccion = conversion();
        } //El movimiento se aleatoriza.
        break;
    case izquierda_s:
        if (*(*(mapa + (enemigo.coord_y - 70) / 5) + ((enemigo.coord_x - 66) / 5) - 1 + 5) != '*' && ((enemigo.coord_x - 66) / 5) - 1 + 5 >= 0 && movimiento_activado && juego_iniciado && !juego_pausado && enemigo.movimiento) {
            enemigo.coord_x -= velocidad;
            enemigo.animacion++;
        } //El fantasma actual se direcciona hacia la izquierda.
        else {
            if (!juego_pausado && enemigo.movimiento) enemigo.direccion = conversion();
        } //El movimiento se aleatoriza.
        break;
    }
}

//Delimita las zonas de teletransporte para los fantasmas.
void definir_teletransporte_fantasmas(char* mapa[], Fantasma& enemigo) {
    if (((enemigo.coord_y - 70) / 5) - 1 == 45 && (((enemigo.coord_x - 66) / 5) + 5) == 1) {
        enemigo.coord_x = (96 - 5) * 5 + 75;
    }
    if (((enemigo.coord_y - 70) / 5) - 1 == 45 && (((enemigo.coord_x - 66) / 5) + 5) == 98) {
        enemigo.coord_x = (2 - 5) * 5 + 75;
    }
    if (((enemigo.coord_y - 70) / 5) - 1 == 11 && (((enemigo.coord_x - 66) / 5) + 5) == 1) {
        enemigo.coord_x = (96 - 5) * 5 + 75;
    }
    if (((enemigo.coord_y - 70) / 5) - 1 == 11 && (((enemigo.coord_x - 66) / 5) + 5) == 98) {
        enemigo.coord_x = (2 - 5) * 5 + 75;
    }
    if (((enemigo.coord_y - 70) / 5) - 1 == 82 && (((enemigo.coord_x - 66) / 5) + 5) == 1) {
        enemigo.coord_x = (96 - 5) * 5 + 75;
    }
    if (((enemigo.coord_y - 70) / 5) - 1 == 82 && (((enemigo.coord_x - 66) / 5) + 5) == 98) {
        enemigo.coord_x = (2 - 5) * 5 + 75;
    }
    if (*(*(mapa + (enemigo.coord_y - 70) / 5) + ((enemigo.coord_x - 66) / 5) + 5) == 'M') enemigo.puerta_cruzada = 1;
}

movimiento_fantasmas conversion() {
    movimiento_fantasmas direccion = arriba_s;
    int x = rand() % 4 + 1;

    switch (x) {
    case 1: direccion = arriba_s; break;
    case 2: direccion = abajo_s; break;
    case 3: direccion = izquierda_s; break;
    case 4: direccion = derecha_s; break;
    }
    return direccion;
}

//Se genera un código para la reanudación de las partidas.
Codigo generar_codigo() {
    Codigo clave;
    char cadena[7] = "";

    for (int i = 0; i < 6; i++) {
        cadena[i] = letras_aleatorias();
    }
    
    strcpy_s(clave.cadena, 7, cadena);

    return clave;
}

//Aleatoriza caracteres alfanuméricos para generar aleatoriamente cada contraseña.
char letras_aleatorias() {
    int x = rand() % (26) + 1;

    switch (x) {
    case 2: return 'B'; break; case 3: return 'C'; break;
    case 4: return '3'; break; case 5: return 'E'; break; case 6: return 'F'; break;
    case 7: return 'G'; break; case 8: return '4'; break; case 9: return 'I'; break;
    case 10: return 'J'; break; case 11: return 'K'; break; case 12: return 'L'; break;
    case 13: return '1'; break; case 14: return '2'; break; case 15: return '6'; break;
    case 16: return 'P'; break; case 17: return 'Q'; break; case 18: return '7'; break;
    case 19: return 'S'; break; case 21: return '9'; break;
    case 22: return '8'; break; case 23: return '2'; break; case 24: return 'X'; break;
    case 25: return '6'; break; case 26: return 'Z'; break; default: return '1';
    }
}

//Permite que los usuarios puedan reanudar sus partidas.
Datos continuar(ALLEGRO_DISPLAY* pantalla) {
    Datos solicitud, auxiliar;
    bool encontrado = false;

    FILE * contrasenias = fopen("contraseñas.dat", "rb");
    if (!contrasenias) contrasenias = fopen("contraseñas.dat", "ab");
    fclose(contrasenias);
    contrasenias = fopen("contraseñas.dat", "rb"); //Se asegura la apertura del archivo para la lectura de contraseñas.

    solicitud = preguntar_nombre(pantalla, 2); //Se solicita el nombre y contraseña a validar.

    fread(&auxiliar, sizeof(Datos), 1, contrasenias);
    while (!feof(contrasenias)) {
        if (strcmp(solicitud.nombre, auxiliar.nombre) == 0 && strcmp(solicitud.codigo, auxiliar.codigo) == 0) {
            encontrado = true;
            break;
        }
        fread(&auxiliar, sizeof(Datos), 1, contrasenias);
    } //Validación de los datos ingresados.

    if (!encontrado) {
        auxiliar.mapa_actual = -1;
    } //Se invalida la contraseña y/o usuario.
    fclose(contrasenias);
    return auxiliar;
}

//Muestra el ranking de las ocho puntuaciones más altas registradas.
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
    } // Matriz para el almacenamiento temporal de los nombres.
    contador = 0;

    if (ftell(registro) > 0) {
        fseek(registro, 0, SEEK_SET);
        fread(&lista, sizeof(Datos), 1, registro);
        while (!feof(registro)) {
            *(valores + contador) = lista.puntaje;
            strcpy_s(nombres[contador], 7, lista.nombre);
            fread(&lista, sizeof(Datos), 1, registro); 
            if (!feof(registro)) contador++;
        } //Extracción de todos los nombres y puntuaciones almacenados.
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
        } //Ordenamiento del ranking.

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
        } //Impresión de las puntuaciones en forma de ranking.
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
            } //Se regresa al menú principal si se presiona "Enter".
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
    delete valores;
    for (int i = 0; i < contador++; i++) {
        delete[] nombres[i];
    }
    delete[] nombres;
    rewind(registro);
    fclose(registro);
}