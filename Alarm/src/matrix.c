#include "matrix.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// LED structure (GRB format)
typedef struct {
    uint8_t G, R, B;
} npLED_t;

static npLED_t leds[LED_COUNT];
static PIO np_pio;
static uint sm;

// Initialize the LED matrix
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

// Set an individual LED color
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_COUNT) {
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

// Set all LEDs to a specific color
void matrix_set_color(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; i++) {
        npSetLED(i, r, g, b);
    }
    matrix_update();
}

// Send the buffer data to LEDs
void matrix_update() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

// Turn off all LEDs
void matrix_clear() {
    matrix_set_color(0, 0, 0);
}

// Blink the LEDs three times when the alarm triggers
void matrix_blink_alarm() {
    for (int i = 0; i < 3; i++) {
        matrix_set_color(100, 100, 100); // Dim white light
        sleep_ms(500);

        matrix_clear();
        sleep_ms(500);
    }
}
