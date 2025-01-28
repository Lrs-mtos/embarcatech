#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

// Inicializa o joystick e os botões
void joystick_init(void);

// Funções para verificar a direção do joystick
bool joystick_left(void);
bool joystick_right(void);
bool joystick_up(void);
bool joystick_down(void);

// Funções para verificar os botões A e B
bool button_a_pressed(void);
bool button_b_pressed(void);

#endif // JOYSTICK_H
