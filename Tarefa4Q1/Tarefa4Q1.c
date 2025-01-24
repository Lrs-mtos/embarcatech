#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Configuração dos pinos
const uint BLUE_LED_PIN= 12;   // Ex.: LED on-board do RP2040 (geralmente GPIO 25)
#define BUTTON_PIN  5   // Botão A 

// Variáveis globais 
static volatile uint32_t press_count = 0;
static volatile bool led_blinking_mode = false; 
static volatile bool button_was_pressed = false; 

// Função de callback para o timer repetitivo
// Faz a varredura do botão e atualiza o contador de pressões
bool timer_callback_button_check(repeating_timer_t *rt) {
    // Lê o estado do pino do botão
    bool button_state = gpio_get(BUTTON_PIN);

    // Se o botão for "ativo em nível baixo", ajuste a lógica conforme necessário
    // Exemplo: se o hardware fechar GND ao apertar, então '0' = pressionado
    // Aqui supondo "0" = pressionado
    if(!button_state) {
        // Se estava solto e agora está pressionado, incrementamos
        if(!button_was_pressed) {
            button_was_pressed = true;
            press_count++;
            printf("Botão pressionado! Contagem = %d\n", press_count);
        }
    } else {
        // Se o botão não está pressionado, reseta flag interna
        button_was_pressed = false;
    }

    // Se chegou a 5 pressões e ainda não está piscando, inicia o modo de piscar
    if(press_count >= 5 && !led_blinking_mode) {
        led_blinking_mode = true;
        press_count = 0;   // zera contagem caso deseje reiniciar depois
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

int main() {
    stdio_init_all();
    printf("Inicializando...\n");

    // Configura o pino do LED como saída
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_put(BLUE_LED_PIN, 0); // LED inicialmente apagado

    // Configura o pino do botão como entrada com pull-up (ou pull-down) interno
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    // Se o seu hardware precisar de pull-down (ou resistor externo),
    // trocar para gpio_pull_down(BUTTON_PIN);

    // Cria um timer repetitivo para verificar o botão a cada 10ms, por exemplo
    repeating_timer_t timer_button;
    add_repeating_timer_ms(10, timer_callback_button_check, NULL, &timer_button);

    // Loop principal
    while(true) {
        // Se estiver no modo de piscar, executa o piscar por 10s e depois volta
        if(led_blinking_mode) {
            printf("Iniciando piscar de 10s a 10Hz...\n");
            blink_led_10s_10hz();
            printf("Fim do piscar.\n");

            // Desativa modo piscando, LED fica apagado e volta ao loop normal
            gpio_put(BLUE_LED_PIN, 0);
            led_blinking_mode = false;
        }
        sleep_ms(50); // pausa rápida para evitar loop muito rápido
    }

    return 0;
}
