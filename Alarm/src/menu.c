#include <stdio.h>
#include "oled.h"
#include "joystick.h"
#include "pico/stdlib.h"  // Necessário para sleep_ms
#include "menu.h"
#include <stdbool.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "buzzer.h"

// Default settings
#define BUZZER_PIN 21  // Change 10 to your actual buzzer GPIO pin


#define DEFAULT_ALARM_HOUR 0
#define DEFAULT_ALARM_MINUTE 0
#define DEFAULT_RINGTONE 0
#define DEFAULT_COLOR 0
#define DEFAULT_BRIGHTNESS 0

// To store user settings

static int alarm_hour = DEFAULT_ALARM_HOUR;
static int alarm_minute = DEFAULT_ALARM_MINUTE;
static int alarm_second = 0;
static int ringtone = DEFAULT_RINGTONE;
static int color = DEFAULT_COLOR;
static int brightness = DEFAULT_BRIGHTNESS;

const char *menu_options[] = {
    "1 Alarm",
    "2 Lighting",
    "3 Ringtone",
    "4 Reset"
};

#define NUM_OPTIONS (sizeof(menu_options) / sizeof(menu_options[0]))
// Variável global para rastrear o contexto atual
static int menu_context = 0; // 0: Menu principal, 1: Configuração de alarme, etc.
// Variável para rastrear a última opção desenhada
static int last_selected_option = -1;
// Variável para verificar se o display deve ser limpo
static bool clear_display = false;

const char *ringtone_options[] = {
    "Iphone",
    "Samsung",
    "Nokia"
};

#define NUM_RINGTONES (sizeof(ringtone_options) / sizeof(ringtone_options[0]))
// Variável global para armazenar a seleção atual do ringtone
static int selected_ringtone = 0; 

const char* color_options[] = {
    "White ",
    "Blue   ",
    "Red    ",
    "Yellow"
};

#define NUM_COLORS (sizeof(color_options) / sizeof(color_options[0]))
// Variável para armazenar a cor selecionada
static int selected_color = 0;

// Alarm state (true = active, false = inactive)
static bool alarm_set = false;

/* 
Função para desenhar o menu principal
Nessa função é feita a lógica de desenho do menu principal.
*/
void draw_menu(int selected_option) {
    if (selected_option != last_selected_option) { 

        // Clear display only when changing selection
        if(clear_display){
            oled_clear();
        }   
        clear_display = true;

        // Fetch the current RTC time
        datetime_t now;
        rtc_get_datetime(&now);

        // Format the RTC time as HH:MM:SS
        char current_time[10];
        snprintf(current_time, sizeof(current_time), "%02d:%02d:%02d", now.hour, now.min, now.sec);

        // Draw the menu options
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected_option) {
                oled_display_text(">", 0, i * 10); // Show selection arrow
            }
            oled_display_text(menu_options[i], 10, i * 10);
            oled_display_text(current_time, 50, 50);  // Show current RTC time
        }
        
        // Draw separator line
        oled_draw_line(0, 40, 120, 40);
        
        // Display "SEL A" and the RTC time
        oled_display_text("SEL A ", 0, 50);

        // Update the last selected option
        last_selected_option = selected_option;
    }
}

void update_time_display() {
    datetime_t now;
    rtc_get_datetime(&now);  // Fetch real-time clock

    // Format RTC time as HH:MM:SS
    char current_time[10];
    snprintf(current_time, sizeof(current_time), "%02d:%02d:%02d", now.hour, now.min, now.sec);

    // Display only the updated time without redrawing the entire menu
    oled_display_text(current_time, 50, 50);
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
        char time_str[6];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", alarm_hour, alarm_minute);
        oled_display_text("Set Alarm:", 10, 5);
        oled_display_text(time_str, 30, 20);
        oled_display_text(editing_hours ? "^" : " ", 35, 35);
        oled_display_text(!editing_hours ? "^" : " ", 75, 35);

        if (joystick_up()) {
            if (editing_hours) {
                alarm_hour = (alarm_hour + 1) % 24;
            } else {
                alarm_minute = (alarm_minute + 1) % 60;
            }
        } else if (joystick_down()) {
            if (editing_hours) {
                alarm_hour = (alarm_hour - 1 + 24) % 24;
            } else {
                alarm_minute = (alarm_minute - 1 + 60) % 60;
            }
        } else if (joystick_left()) {
            editing_hours = true;
        } else if (joystick_right()) {
            editing_hours = false;
        }

        // Verifica botões A e B
        if (button_a_pressed()) {
            alarm_set = true; // Ativa o alarme
            sleep_ms(300); // Delay para evitar múltiplos pressionamentos
            printf("Alarme configurado para %02d:%02d\n", alarm_hour, alarm_minute);
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

void check_alarm() {
    if (!alarm_set) return; // Only check if an alarm is set

    datetime_t now;
    rtc_get_datetime(&now);

if (alarm_set && now.hour == alarm_hour && now.min == alarm_minute) {
        printf("ALARM TRIGGERED at %02d:%02d!\n", now.hour, now.min);

        oled_clear();
        oled_display_text("ALARM!!!", 30, 20);
        oled_display_text("Press B to Stop", 10, 40);

        // Play selected ringtone
        while (1) {
            play_ringtone(selected_ringtone);

            if (button_b_pressed()) {
                stop_buzzer(); // Stop ringtone
                printf("Alarm Stopped\n");
                oled_clear();
                oled_display_text("Alarm Stopped", 10, 20);
                sleep_ms(1000);
                oled_clear();
                draw_menu(-1);
                alarm_set = false; // Disable alarm
                break;
            }
        }
    }
}


void configure_ringtone() {
    printf("Configurando o ringtone...\n");
    oled_clear();

    while (1) {
        

        oled_display_text("Select Ringtone:", 0, 0);

        for (int i = 0; i < NUM_RINGTONES; i++) {
            if (i == selected_ringtone) {
                oled_display_text(">", 0, (i * 10) + 10); // Indica a opção selecionada
            }
            oled_display_text(ringtone_options[i], 10, (i * 10) + 10);
        }

        // Navegação com o joystick
        if (joystick_down()) {
            selected_ringtone = (selected_ringtone + 1) % NUM_RINGTONES;
            oled_clear();  // Limpa a tela antes de redesenha
        } else if (joystick_up()) {
            selected_ringtone = (selected_ringtone - 1 + NUM_RINGTONES) % NUM_RINGTONES;
            oled_clear();  // Limpa a tela antes de redesenha
        }

        // Confirma a seleção com o botão A
        if (button_a_pressed()) {
            sleep_ms(300);
            printf("Ringtone selecionado: %s\n", ringtone_options[selected_ringtone]);
            menu_context = 0; // Voltar ao menu principal
            oled_clear();
            oled_display_text("Ringtone\nSelected:", 0, 20);
            oled_display_text(ringtone_options[selected_ringtone], 0, 40);
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


void configure_lighting() {
    printf("Configurando iluminação...\n");
    oled_clear();

    // Desenha linha separadora
    oled_draw_line(0, 43, 120, 43);  
    oled_display_text("Move Joystick", 0, 50);

    while (1) {
        oled_display_text("Adjust Lighting", 0, 0);

        // Mostra a barra de brilho
        char brightness_str[16];
        snprintf(brightness_str, sizeof(brightness_str), "Brightness: %d%%", brightness);
        oled_display_text(brightness_str, 0, 20);

        // Mostra a cor selecionada
        char color_str[16];
        snprintf(color_str, sizeof(color_str), "Color: %s", color_options[selected_color]);
        oled_display_text(color_str, 0, 30);

        // Ajuste do brilho
        if (joystick_up()) {
            brightness = (brightness + 10 > 100) ? 100 : brightness + 10;
        } else if (joystick_down()) {
            brightness = (brightness - 10 < 0) ? 0 : brightness - 10;
        }

        // Troca de cor
        if (joystick_right()) {
            selected_color = (selected_color + 1) % NUM_COLORS;
        } else if (joystick_left()) {
            selected_color = (selected_color - 1 + NUM_COLORS) % NUM_COLORS;
        }

        // Confirma a seleção com o botão A
        if (button_a_pressed()) {
            sleep_ms(300);
            printf("Iluminação ajustada: %d%%, Cor: %s\n", brightness, color_options[selected_color]);
            menu_context = 0; // Voltar ao menu principal
            oled_clear();

            oled_display_text("Lighting:", 0, 10);
            oled_display_text(color_options[selected_color], 75, 10);
            oled_display_text(brightness_str, 0, 25);
            sleep_ms(1500);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        // Retorna ao menu principal com o botão B
        if (button_b_pressed()) {
            printf("Voltando ao menu principal sem alterar iluminação.\n");
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        sleep_ms(200); // Delay para suavizar a navegação
    }
}

void reset_settings() {
    printf("Entrando no reset de configurações...\n");
    oled_clear();

    int confirm_selection = 0;  // 0 = No, 1 = Yes

    while (1) {
        oled_display_text("Reset Settings?", 10, 5);
        oled_display_text("Yes", 30, 20);
        oled_display_text("No", 30, 30);

        // Show selection arrow
        oled_display_text(confirm_selection == 0 ? ">" : " ", 20, 20);
        oled_display_text(confirm_selection == 1 ? ">" : " ", 20, 30);

        // Navigate options
        if (joystick_down() || joystick_up()) {
            confirm_selection = !confirm_selection; // Toggle selection
        }

        // Confirm selection
        if (button_a_pressed()) {
            if (confirm_selection == 0) { // Yes selected
                // Reset all values to default
                alarm_hour = DEFAULT_ALARM_HOUR;
                alarm_minute = DEFAULT_ALARM_MINUTE;
                selected_ringtone = DEFAULT_RINGTONE;
                brightness = DEFAULT_BRIGHTNESS;
                selected_color = DEFAULT_COLOR;

                printf("Configurações resetadas para o padrão!\n");

                oled_clear();
                oled_display_text("Settings Reset!", 10, 20);
                clear_display = false;
                sleep_ms(1000);
            }
            break; // Exit reset menu
        }

        // Cancel and return to menu
        if (button_b_pressed()) {
            printf("Reset cancelado!\n");
            clear_display = false;
            break;
        }

        sleep_ms(200); // Smooth navigation
    }

    // Return to main menu
    menu_context = 0;
    oled_clear();
    draw_menu(-1);
}


void menu_navigation() {
    static int selected_option = 0;
    //while(1){
        if (menu_context == 0) { // Menu principal
            draw_menu(selected_option);

            if (joystick_down()) {
                selected_option = (selected_option + 1) % NUM_OPTIONS;
                printf("Selecionado: %s\n", menu_options[selected_option]);
            } else if (joystick_up()) {
                selected_option = (selected_option - 1 + NUM_OPTIONS) % NUM_OPTIONS;
                printf("Selecionado: %s\n", menu_options[selected_option]);
            } else if (button_a_pressed()) {
                printf("Selecionado: %s\n", menu_options[selected_option]);

                switch (selected_option) {
                    case 0: // Configuração do alarme
                        printf("Entrando na configuração do alarme\n");
                        menu_context = 1;
                        configure_alarm();
                        break;
                    case 1:
                        printf("Entrando na configuração de iluminação\n");
                        menu_context = 2;
                        configure_lighting();
                        break;
                    case 2:
                        printf("Entrando na configuração de ringtone\n");
                        menu_context = 2;
                        configure_ringtone();
                        break;
                    case 3:
                        printf("Redefinindo configurações\n");
                        menu_context = 2;
                        reset_settings();
                        break;
                }
            }

            if (button_b_pressed()) {
                printf("Botão B ignorado no menu principal\n");
            }
        }
    //}
}


