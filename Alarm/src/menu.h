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

#endif // MENU_H
