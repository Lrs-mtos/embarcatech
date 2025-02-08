#include "joystick.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include <stdio.h>

// Pinos do joystick
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26

// Limites para identificar os movimentos do joystick
#define JOYSTICK_THRESHOLD_LEFT 500
#define JOYSTICK_THRESHOLD_RIGHT 3900
#define JOYSTICK_THRESHOLD_UP 3900
#define JOYSTICK_THRESHOLD_DOWN 500

// Pinos dos botões
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Debounce time (milliseconds)
#define DEBOUNCE_TIME_MS 250

// Flags de botão pressionado
volatile bool button_a_flag = false;
volatile bool button_b_flag = false;

// Last time each button was pressed
volatile uint32_t last_press_a = 0;
volatile uint32_t last_press_b = 0;

// Interrupt handler for button presses
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = time_us_32() / 1000; // Get current time in ms

    if (gpio == BUTTON_A_PIN) {
        if (now - last_press_a > DEBOUNCE_TIME_MS) { // Debounce check
            last_press_a = now;
            button_a_flag = true;
            printf("Botão A pressionado (IRQ)\n");
        }
    }

    if (gpio == BUTTON_B_PIN) {
        if (now - last_press_b > DEBOUNCE_TIME_MS) { // Debounce check
            last_press_b = now;
            button_b_flag = true;
            printf("Botão B pressionado (IRQ)\n");
        }
    }
}

void joystick_init(void) {
    sleep_ms(500);
    printf("Configurando o joystick...\n");
    
    // Configura os canais ADC para os eixos X e Y
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Configura os botões como entrada com pull-up
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    printf("Joystick configurado...\n");
    sleep_ms(1000);
}

// Funções para verificar os estados do joystick
bool joystick_left(void) {
    adc_select_input(1);
    uint16_t value = adc_read();
    return value < JOYSTICK_THRESHOLD_LEFT;
}

bool joystick_right(void) {
    adc_select_input(1);
    uint16_t value = adc_read();
    return value > JOYSTICK_THRESHOLD_RIGHT;
}

bool joystick_up(void) {
    adc_select_input(0);
    uint16_t value = adc_read();
    return value > JOYSTICK_THRESHOLD_UP;
}

bool joystick_down(void) {
    adc_select_input(0);
    uint16_t value = adc_read();
    return value < JOYSTICK_THRESHOLD_DOWN; 
}

// Funções para verificar os estados dos botões
bool button_a_pressed(void) {
    if (button_a_flag) {
        button_a_flag = false;
        return true;
    }
    return false;
}

bool button_b_pressed(void) {
    if (button_b_flag) {
        button_b_flag = false;
        return true;
    }
    return false;
}
