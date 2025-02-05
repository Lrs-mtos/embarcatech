#include "matrix.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

typedef struct {
    uint8_t G, R, B; // Pixel structure for NeoPixel (WS2812B)
} npLED_t;

npLED_t leds[LED_COUNT];        // Stores **current** LED colors
npLED_t original_leds[LED_COUNT]; // Stores **original** colors for brightness control

PIO np_pio;
uint sm;
uint brightness_level = 255; // Default brightness (100%)

void matrix_init() {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2818b_program_init(np_pio, sm, offset, LED_PIN, 800000.f);
    matrix_clear();
}

//  **Set LED color without modifying brightness directly**
void matrix_set_color(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; i++) {
        original_leds[i].R = r;
        original_leds[i].G = g;
        original_leds[i].B = b;
    }
    matrix_apply_brightness();
}

//  **Apply brightness scaling correctly**
void matrix_apply_brightness() {
    for (uint i = 0; i < LED_COUNT; i++) {
        leds[i].R = (original_leds[i].R * brightness_level) / 255;
        leds[i].G = (original_leds[i].G * brightness_level) / 255;
        leds[i].B = (original_leds[i].B * brightness_level) / 255;
    }
    matrix_update();
}

//  **Adjust brightness without modifying stored colors**
void matrix_set_brightness(uint8_t brightness) {
    brightness_level = brightness;
    matrix_apply_brightness(); // Apply brightness without modifying original colors
}

//  **Update LED matrix with new values**
void matrix_update() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

//  **Clear all LEDs and reset brightness**
void matrix_clear() {
    for (uint i = 0; i < LED_COUNT; i++) {
        original_leds[i].R = 0;
        original_leds[i].G = 0;
        original_leds[i].B = 0;
    }
    matrix_set_brightness(0);  // Ensure brightness is zero
    matrix_update();
}

//  **Turn off LEDs completely**
void matrix_off() {
    matrix_clear();
}
