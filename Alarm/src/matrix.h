#ifndef MATRIX_H
#define MATRIX_H

#include "pico/stdlib.h"

// LED Matrix definitions
#define LED_COUNT 25
#define LED_PIN 7

// Function prototypes
void matrix_init();
void matrix_set_color(uint8_t r, uint8_t g, uint8_t b);
void matrix_set_brightness(uint8_t brightness);
void matrix_apply_brightness();
void matrix_clear();
void matrix_update();
void matrix_off();

#endif
