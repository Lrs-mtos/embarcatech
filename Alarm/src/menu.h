#ifndef MENU_H
#define MENU_H

#include <stdint.h>

// Inicializa o menu
void menu_init(void);

// Navega no menu principal
void menu_navigation(void);

// Desenha o menu principal
void draw_menu(int selected_option);

// Configura o alarme
void configure_alarm();

// Configura o ringtone
void configure_ringtone();

// Configura a iluminação
void configure_lighting();

// Faz o reset das configurações
void reset_settings();

#endif // MENU_H
