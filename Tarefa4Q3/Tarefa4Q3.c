#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Definições para UART0 e UART1
#define UART0_ID uart0
#define UART1_ID uart1
#define BAUD_RATE 115200

// Definição dos pinos para UART0
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

// Definição dos pinos para UART1
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

// Pinos do LED RGB
#define LED_R_PIN 11
#define LED_G_PIN 12
#define LED_B_PIN 13

// Função para configurar o LED RGB
void set_rgb_color(bool r, bool g, bool b) {
    gpio_put(LED_R_PIN, r); // Vermelho
    gpio_put(LED_G_PIN, g); // Verde
    gpio_put(LED_B_PIN, b); // Azul
}

int main() {
    // Inicializa o console USB e configura GPIOs
    stdio_init_all();
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Configuração da UART0
    uart_init(UART0_ID, BAUD_RATE);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART); // Configura TX para UART0
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART); // Configura RX para UART0

    // Configuração da UART1
    uart_init(UART1_ID, BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART); // Configura TX para UART1
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART); // Configura RX para UART1

    printf("UART0 -> UART1 Comunicação inicializada.\n");

    while (true) {
        // Solicita entrada de dados via console USB
        printf("Digite um caractere para enviar: ");
        char input_char;
        scanf(" %c", &input_char);

        // Acende a cor azul para indicar envio
        set_rgb_color(1, 0, 0);
        sleep_ms(50);

        // Envia o dado de UART0 para UART1
        uart_putc(UART1_ID, input_char);
        printf("Dado '%c' enviado via UART0 -> UART1.\n", input_char);


        // Aguarda o dado ser recebido pela UART1
        if (uart_is_readable(UART1_ID)) {
            char received_char = uart_getc(UART1_ID);

            // Exibe o dado recebido no console USB
            printf("Dado recebido pela UART1: '%c'\n", received_char);
        }

        // Apaga o LED após processar
        set_rgb_color(0, 0, 0);
    }

    return 0;
}
