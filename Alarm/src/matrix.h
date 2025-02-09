#ifndef MATRIX_H
#define MATRIX_H

#include "pico/stdlib.h"
#include <stdint.h>

#define LED_COUNT 25
#define LED_PIN 7

// Function prototypes
void matrix_init();
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b);
void matrix_set_color(uint8_t r, uint8_t g, uint8_t b);
void matrix_update();
void matrix_clear();
void matrix_blink_alarm();

#endif // MATRIX_H
