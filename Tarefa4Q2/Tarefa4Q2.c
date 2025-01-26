#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Configuração dos pinos
const uint32_t BLUE_LED_PIN = 12; 
#define BUTTON_A_PIN  5   // Botão A 
#define BUTTON_B_PIN  6   // Botão B 

// Variáveis globais 
static volatile uint32_t pressA_count = 0;
static volatile bool led_A_blinking_mode = false;
static volatile bool buttonA_was_pressed = false; 
static volatile bool buttonB_was_pressed = false; 
static volatile bool change_to_1hz = false; // Indica se deve mudar a frequência para 1Hz

// Função de callback para o timer repetitivo
bool timer_callback_button_check(repeating_timer_t *rt) {
    // Lê o estado do pino do botão
    bool buttonA_state = gpio_get(BUTTON_A_PIN);
    bool buttonB_state = gpio_get(BUTTON_B_PIN);

    // '0' = pressionado
    if (!buttonA_state) {
        // Se estava solto e agora está pressionado, incrementa
        if (!buttonA_was_pressed) {
            buttonA_was_pressed = true;
            pressA_count++;
            printf("Botão A pressionado! Contagem = %d\n", pressA_count);
        }
    } else {
        // Se o botão não está pressionado, reseta flag interna
        buttonA_was_pressed = false;
    }

    // Se chegou a 5 pressões e ainda não está piscando, inicia o modo de piscar
    if (pressA_count >= 5 && !led_A_blinking_mode) {
        led_A_blinking_mode = true;
        change_to_1hz = false; // Reseta a frequência para 10Hz inicialmente
        pressA_count = 0;      // Zera contagem caso deseje reiniciar depois
    }

    if (!buttonB_state) {
        // Se estava solto e agora está pressionado
        if (!buttonB_was_pressed) {
            buttonB_was_pressed = true;
            if (led_A_blinking_mode) {
                change_to_1hz = true; // Solicita a mudança para 1Hz
                printf("Botão B pressionado! Mudando frequência para 1Hz.\n");
            }
        }
    } else {
        // Se o botão não está pressionado, reseta flag interna
        buttonB_was_pressed = false;
    }

    return true; // Para continuar chamando o timer
}

// Função para piscar o LED com mudança de frequência
void dynamic_blink_led_10s(void) {
    const uint32_t total_time_ms = 10000; // 10 segundos
    uint32_t elapsed_time = 0;
    uint32_t period = 100; // Começa com 10Hz (100ms)
    uint32_t current_second = 0;

    while (elapsed_time < total_time_ms) {
        // Alterna o LED
        gpio_xor_mask(1u << BLUE_LED_PIN);
        sleep_ms(period);

        // Atualiza o tempo decorrido
        elapsed_time += period;

        // Checa se um segundo inteiro passou
        if (elapsed_time / 1000 > current_second) {
            current_second = elapsed_time / 1000;
            printf("Tempo decorrido: %d segundos\n", current_second);
        }

        // Se o botão B foi pressionado, muda a frequência para 1Hz
        if (change_to_1hz) {
            period = 1000; // 1Hz = 1000ms
            change_to_1hz = false; // Reseta o estado
        }
    }

    // Apaga o LED após terminar
    gpio_put(BLUE_LED_PIN, 0);
}

int main() {
    stdio_init_all();
    printf("Inicializando...\n");

    // Configura o pino do LED como saída
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_put(BLUE_LED_PIN, 0); // LED inicialmente apagado

    // Configura o pino do botão A como entrada com pull-up interno
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    // Configura o pino do botão B como entrada com pull-up interno
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Cria um timer repetitivo para verificar os botões a cada 10ms
    repeating_timer_t timer_button;
    add_repeating_timer_ms(10, timer_callback_button_check, NULL, &timer_button);

    // Loop principal
    while (true) {
        // Se estiver no modo de piscar, executa o piscar dinamicamente
        if (led_A_blinking_mode) {
            printf("Iniciando piscar de 10s a 10Hz...\n");
            dynamic_blink_led_10s();
            printf("Fim do piscar.\n");

            // Desativa modo piscando, LED fica apagado e volta ao loop normal
            gpio_put(BLUE_LED_PIN, 0);
            led_A_blinking_mode = false;
        }

        sleep_ms(50); // Pausa rápida para evitar loop muito rápido
    }

    return 0;
}
