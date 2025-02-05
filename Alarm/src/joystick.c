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

// Flags de botão pressionado
volatile bool button_a_flag = false;
volatile bool button_b_flag = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A_PIN) {
        sleep_ms(50); // Debounce
        button_a_flag = true;
        printf("Botão A pressionado (IRQ)\n");
    }
    if (gpio == BUTTON_B_PIN) {
        sleep_ms(50); // Debounce
        button_b_flag = true;
        printf("Botão B pressionado (IRQ)\n");
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
    adc_select_input(1); // Agora para o eixo X
    uint16_t value = adc_read();
    //printf("Valor do joystick_left: %d\n", value);
    return value < JOYSTICK_THRESHOLD_LEFT;
}

bool joystick_right(void) {
    adc_select_input(1); // Agora para o eixo X
    uint16_t value = adc_read();
    //printf("Valor do joystick_right: %d\n", value);
    return value > JOYSTICK_THRESHOLD_RIGHT;
}

bool joystick_up(void) {
    adc_select_input(0); // Agora para o eixo Y
    uint16_t value = adc_read();
    //printf("Valor do joystick_up: %d\n", value);
    return value > JOYSTICK_THRESHOLD_UP;
}

bool joystick_down(void) {
    adc_select_input(0); // Agora para o eixo Y
    uint16_t value = adc_read();
    //printf("Valor do joystick_down: %d\n", value);
    return value < JOYSTICK_THRESHOLD_DOWN; 
}

// Funções para verificar os estados dos botões
bool button_a_pressed(void) {
    if (button_a_flag) {
        button_a_flag = false; // Limpa a flag
        return true;
    }
    return false;
}

bool button_b_pressed(void) {
    if (button_b_flag) {
        button_b_flag = false; // Limpa a flag
        return true;
    }
    return false;
}
