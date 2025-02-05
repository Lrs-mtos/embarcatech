#include <stdio.h>
#include "oled.h"
#include "joystick.h"
#include "pico/stdlib.h"  // Necessário para sleep_ms
#include "menu.h"
#include <stdbool.h>


const char *menu_options[] = {
    "1 Alarme",
    "2 Iluminacao",
    "3 Ringtone",
    "4 Redefinir"
};

#define NUM_OPTIONS (sizeof(menu_options) / sizeof(menu_options[0]))

const char *ringtone_options[] = {
    "1 Iphone",
    "2 Samsung",
    "3 Nokia",
    "4 Motorola"
};

#define NUM_RINGTONES (sizeof(ringtone_options) / sizeof(ringtone_options[0]))

// Variável global para armazenar a seleção atual do ringtone
static int selected_ringtone = 0; 
// Variável global para rastrear o contexto atual
static int menu_context = 0; // 0: Menu principal, 1: Configuração de alarme, etc.
// Variável para rastrear a última opção desenhada
static int last_selected_option = -1;
// Variável para verificar se o display deve ser limpo
static bool clear_display = false;


/* 
Função para desenhar o menu principal
Nessa função é feita a lógica de desenho do menu principal.
*/
void draw_menu(int selected_option) {
    if (selected_option != last_selected_option) { 

        // Limpa o display somente ao mudar a seleção
        if(clear_display){
            oled_clear();
        }   
        clear_display = true;

        // Desenha o menu com a nova seleção
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected_option) {
                oled_display_text(">", 0, i * 10); // Mostra seta na seleção
            }
            oled_display_text(menu_options[i], 10, i * 10);
        }
        
        // Desenha linha separadora
        oled_draw_line(0, 40, 120, 40);  
        oled_display_text("Press A ", 0, 50);

        // Atualiza a variável de rastreamento
        last_selected_option = selected_option;
    }
}


/* Função para configurar o alarme. Nessa função é feita a 
lógica que seleciona as horas e minutos do alarme. */

void configure_alarm() {
    printf("Configurando o alarme...\n");
    int hours = 0;
    int minutes = 0;
    bool editing_hours = true; // Define se está editando horas ou minutos
    oled_clear();

    // Desenha linha separadora
    oled_draw_line(0, 40, 120, 40);  
    oled_display_text("Press A ", 0, 50);

    while (1) {
        // Mostra o horário atual
        
        char time_str[6];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);
        oled_display_text("Configure Alarme", 0, 0);
        oled_display_text(time_str, 30, 20);

        if (editing_hours) {
            oled_display_text("^", 35, 35); // Indica que está editando horas
        } else {
            oled_display_text("^", 75, 35); // Indica que está editando minutos
        }

        // Verifica movimento do joystick
        if (joystick_up()) {
            if (editing_hours) {
                hours = (hours + 1) % 24; // Incrementa as horas, máximo 23
            } else {
                minutes = (minutes + 1) % 60; // Incrementa os minutos, máximo 59
            }
        } else if (joystick_down()) {
            if (editing_hours) {
                hours = (hours - 1 + 24) % 24; // Decrementa as horas
            } else {
                minutes = (minutes - 1 + 60) % 60; // Decrementa os minutos
            }
        } else if (joystick_left()) {
            editing_hours = true; // Alterna para edição das horas
        } else if (joystick_right()) {
            editing_hours = false; // Alterna para edição dos minutos
        }

        // Verifica botões A e B
        if (button_a_pressed()) {
            sleep_ms(300); // Delay para evitar múltiplos pressionamentos
            printf("Alarme configurado para %02d:%02d\n", hours, minutes);
            ///////////////////////passar esse dado para ma string para passar pro display_text:
            menu_context = 0; // Volta para o menu principal
            oled_clear();
            oled_display_text("Alarme\nconfigurado", 0, 25);
            sleep_ms(1000);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            // Aqui você pode salvar o horário do alarme em uma variável global ou memória
            break; // Sai do menu de configuração
        } else if (button_b_pressed()) {
            printf("Voltando ao menu principal sem configurar o alarme\n");
            menu_context = 0; // Volta para o menu principal
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break; // Volta ao menu principal sem salvar
        }

        sleep_ms(200); // Evita múltiplos comandos rápidos
    }
}


void configure_ringtone() {
    printf("Configurando o ringtone...\n");
    oled_clear();

    // Desenha linha separadora
    oled_draw_line(0, 40, 120, 40);  
    oled_display_text("Press A", 0, 50);

    while (1) {

        for (int i = 0; i < NUM_RINGTONES; i++) {
            if (i == selected_ringtone) {
                oled_display_text(">", 0, (i * 10)); // Indica a opção selecionada
            }
            oled_display_text(ringtone_options[i], 10, i * 10);
        }

        // Navegação com o joystick
        if (joystick_down()) {
            selected_ringtone = (selected_ringtone + 1) % NUM_RINGTONES;
        } else if (joystick_up()) {
            selected_ringtone = (selected_ringtone - 1 + NUM_RINGTONES) % NUM_RINGTONES;
        }

        // Confirma a seleção com o botão A
        if (button_a_pressed()) {
            sleep_ms(300);
            printf("Ringtone selecionado: %s\n", ringtone_options[selected_ringtone]);
            menu_context = 0; // Voltar ao menu principal
            oled_clear();
            oled_display_text("Ringtone\nSelected", 0, 30);
            sleep_ms(1000);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        // Retorna ao menu principal com o botão B
        if (button_b_pressed()) {
            printf("Voltando ao menu principal sem alterar o ringtone.\n");
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        sleep_ms(200); // Delay para suavizar a navegação
    }
}



void menu_navigation() {
    int selected_option = 0;
    printf("Navegando no menu...\n");
    while (1) {
        if (menu_context == 0) { // Menu principal
            draw_menu(selected_option);

            if (joystick_down()) {
                selected_option = (selected_option + 1) % NUM_OPTIONS; // Próxima opção
                printf("Selecionado: %s\n", menu_options[selected_option]);
            } else if (joystick_up()) {
                selected_option = (selected_option - 1 + NUM_OPTIONS) % NUM_OPTIONS; // Opção anterior
                printf("Selecionado: %s\n", menu_options[selected_option]);
            } else if (button_a_pressed()) {
                printf("Selecionado: %s\n", menu_options[selected_option]);
                
                switch (selected_option) {
                    case 0: // Configuração do alarme
                        printf("Entrando na configuração do alarme\n");
                        menu_context = 1; // Define o contexto como submenu
                        configure_alarm();
                        break;
                    case 1:
                        printf("Entrando na configuração de iluminação\n");
                        break;
                    case 2:
                        printf("Entrando na configuração de ringtone\n");
                        menu_context = 2; // Define o contexto como submenu
                        configure_ringtone();
                        break;
                    case 3:
                        printf("Redefinindo configurações\n");
                        break;
                }
            }

            // Ignorar o botão B no menu principal
            if (button_b_pressed()) {
                printf("Botão B ignorado no menu principal\n");
            }

        } else {
            // Se estiver em outro submenu, o comportamento é gerenciado lá
        }

        sleep_ms(200); // Delay para suavizar a navegação
    }
}

