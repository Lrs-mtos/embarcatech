#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Configuração dos pinos
const uint BLUE_LED_PIN= 12;
const uint RED_LED_PIN = 11;
#define BUTTON_A_PIN  5   // Botão A 
#define BUTTON_B_PIN  6   // Botão B 

// Variáveis globais 
static volatile uint32_t pressA_count = 0;
static volatile uint32_t pressB_count = 0;
static volatile bool led_A_blinking_mode = false;
static volatile bool led_B_blinking_mode = false; 
static volatile bool buttonA_was_pressed = false; 
static volatile bool buttonB_was_pressed = false; 

// Função de callback para o timer repetitivo
// Faz a varredura do botão e atualiza o contador de pressões
bool timer_callback_button_check(repeating_timer_t *rt) {
    // Lê o estado do pino do botão
    bool buttonA_state = gpio_get(BUTTON_A_PIN);
    bool buttonB_state = gpio_get(BUTTON_B_PIN);

    // Se o botão for "ativo em nível baixo", ajuste a lógica conforme necessário
    // Exemplo: se o hardware fechar GND ao apertar, então '0' = pressionado
    // Aqui supondo "0" = pressionado
    if(!buttonA_state) {
        // Se estava solto e agora está pressionado, incrementamos
        if(!buttonA_was_pressed) {
            buttonA_was_pressed = true;
            pressA_count++;
            printf("Botão A pressionado! Contagem = %d\n", pressA_count);
        }
    } else {
        // Se o botão não está pressionado, reseta flag interna
        buttonA_was_pressed = false;
    }

    // Se chegou a 5 pressões e ainda não está piscando, inicia o modo de piscar
    if(pressA_count >= 5 && !led_A_blinking_mode) {
        led_A_blinking_mode = true;
        pressA_count = 0;   // zera contagem caso deseje reiniciar depois
    }

    if(!buttonB_state) {
        // Se estava solto e agora está pressionado, incrementamos
        if(!buttonB_was_pressed) {
            buttonB_was_pressed = true;
            pressB_count++;
            printf("Botão B pressionado! Contagem = %d\n", pressB_count);
        }
    } else {
        // Se o botão não está pressionado, reseta flag interna
        buttonB_was_pressed = false;
    }

    // Se chegou a 5 pressões e ainda não está piscando, inicia o modo de piscar
    if(pressB_count >= 5 && !led_B_blinking_mode) {
        led_B_blinking_mode = true;
        pressB_count = 0;   // zera contagem caso deseje reiniciar depois
    }

    return true; // para continuar chamando o timer
}

// Função para piscar o LED por 10 segundos a 10Hz (bloco "sincronamente" no loop)
void blink_led_10s_10hz(void) {
    // Precisamos de 10 segundos piscando a 10Hz -> 100 toggles
    // Frequência de 10Hz = período de 100 ms
    const uint32_t total_toggles = 100;
    for(uint32_t i = 0; i < total_toggles; i++) {
        gpio_xor_mask(1u << BLUE_LED_PIN);  // Toggle do LED
        sleep_ms(100);                 // período de 100 ms (10Hz)
    }
}

// Função para piscar o LED por 10 segundos a 1HZ (bloco "sincronamente" no loop)
void blink_led_10s_1hz(void) {
    // Precisamos de 10 segundos piscando a 1Hz -> 10 toggles
    // Frequência de 1Hz = período de 1000 ms
    const uint32_t total_toggles = 10;
    for(uint32_t i = 0; i < total_toggles; i++) {
        gpio_xor_mask(1u << RED_LED_PIN);  // Toggle do LED
        sleep_ms(1000);                 // período de 1000 ms (1Hz)
    }
}

int main() {
    stdio_init_all();
    printf("Inicializando...\n");

    // Configura o pino do LED como saída
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_put(BLUE_LED_PIN, 0); // LED inicialmente apagado

    // Configura o pino do LED como saída
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0); // LED inicialmente apagado

    // Configura o pino do botão A como entrada com pull-up (ou pull-down) interno
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    // Configura o pino do botão B como entrada com pull-up (ou pull-down) interno
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Cria um timer repetitivo para verificar o botão a cada 10ms, por exemplo
    repeating_timer_t timer_button;
    add_repeating_timer_ms(10, timer_callback_button_check, NULL, &timer_button);

    // Loop principal
    while(true) {
        // Se estiver no modo de piscar, executa o piscar por 10s e depois volta
        if(led_A_blinking_mode) {
            printf("Iniciando piscar de 10s a 10Hz...\n");
            blink_led_10s_10hz();
            printf("Fim do piscar.\n");

            // Desativa modo piscando, LED fica apagado e volta ao loop normal
            gpio_put(BLUE_LED_PIN, 0);
            led_A_blinking_mode = false;
        }

        if(led_B_blinking_mode) {
            printf("Iniciando piscar de 10s a 1Hz...\n");
            blink_led_10s_1hz();
            printf("Fim do piscar.\n");

            // Desativa modo piscando, LED fica apagado e volta ao loop normal
            gpio_put(RED_LED_PIN, 0);
            led_B_blinking_mode = false;
        }
        sleep_ms(50); // pausa rápida para evitar loop muito rápido
    }

    return 0;
}
