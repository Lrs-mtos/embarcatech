#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

// Initialize joystick and buttons
void joystick_init(void);

// Joystick movement detection
bool joystick_left(void);
bool joystick_right(void);
bool joystick_up(void);
bool joystick_down(void);

// Button press detection
bool button_a_pressed(void);
bool button_b_pressed(void);

#endif // JOYSTICK_H
