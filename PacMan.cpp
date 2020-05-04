#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <stdio.h>

void inicializar_graficos();

int main(int argc, char** argv) {
    srand(time(NULL));
    inicializar_graficos();
    
    enum {abajo = 0, arriba = 4, izquierda = 8, derecha = 12};
    char nombre_archivo[30], numero [3];
    int coord_x = 300, coord_y = 300, velocidad = 4, animacion = 0, direccion = abajo, direccion_previa = abajo;
    bool terminado = false;

    ALLEGRO_DISPLAY* pantalla = al_create_display(600, 600);
    ALLEGRO_FONT* formato = al_load_ttf_font("04B_30__.ttf", 17, NULL);
    ALLEGRO_BITMAP* pacman[24];
    ALLEGRO_EVENT_QUEUE* fila_evento = al_create_event_queue();
    ALLEGRO_TIMER* temporizador = al_create_timer(1.0 / 15);

    for (int i = 0; i < 24; i++) {
        strcpy_s(nombre_archivo, 30, "img/pacman/pacman");
        sprintf_s(numero, 3, "%i", i + 1);
        strcat_s(nombre_archivo, 30, numero);
        strcat_s(nombre_archivo, 30, ".png");
        pacman[i] = al_load_bitmap(nombre_archivo);
        al_convert_mask_to_alpha(pacman[i], al_map_rgb(0, 0, 0));
    }

    al_register_event_source(fila_evento, al_get_keyboard_event_source());
    al_register_event_source(fila_evento, al_get_timer_event_source(temporizador));
   
    al_start_timer(temporizador);

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
            switch(direccion) {
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

        if(animacion > direccion + 3 || direccion != direccion_previa) animacion = direccion;
        al_draw_bitmap(pacman[animacion], coord_x, coord_y, NULL);
        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));
    }

    al_destroy_display(pantalla);
    al_destroy_font(formato);
    for (int i = 0; i < 24; i++) {
        al_destroy_bitmap(pacman[i]);
    }
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
