#include "matrix.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

typedef struct {
    uint8_t G, R, B;
} npLED_t;

static npLED_t leds[LED_COUNT];
static PIO np_pio;
static uint sm;

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

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (index < LED_COUNT) {
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

void matrix_set_color(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; i++) {
        npSetLED(i, r, g, b);
    }
    matrix_update();
}

void matrix_update() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

void matrix_clear() {
    matrix_set_color(0, 0, 0);
}

void matrix_blink_alarm() {
    for (int i = 0; i < 3; i++) {  
        for (uint j = 0; j < LED_COUNT; j++) {
            npSetLED(j, 100, 100, 100);  // Use dim white light
        }
        matrix_update();
        sleep_ms(500);

        matrix_clear();  
        matrix_update();
        sleep_ms(500);
    }
}
