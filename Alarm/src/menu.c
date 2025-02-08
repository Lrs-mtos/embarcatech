#include <stdio.h>
#include "oled.h"
#include "joystick.h"
#include "pico/stdlib.h"  // Necessário para sleep_ms
#include "menu.h"
#include <stdbool.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "buzzer.h"
#include "matrix.h"

// Default settings
#define BUZZER_PIN 21  // Change 10 to your actual buzzer GPIO pin

#define DEFAULT_ALARM_HOUR 12
#define DEFAULT_ALARM_MINUTE 0
#define DEFAULT_RINGTONE 0
#define DEFAULT_COLOR 0
#define DEFAULT_BRIGHTNESS 10

// To store user settings

static int alarm_hour = DEFAULT_ALARM_HOUR;
static int alarm_minute = DEFAULT_ALARM_MINUTE;
static int alarm_second = 0;
static int ringtone = DEFAULT_RINGTONE;
static int color = DEFAULT_COLOR;
static int brightness = DEFAULT_BRIGHTNESS;

const char *menu_options[] = {
    "1 Alarm",
    "2 Ringtone",
    "3 Reset"
};

#define NUM_OPTIONS (sizeof(menu_options) / sizeof(menu_options[0]))
// Variável global para rastrear o contexto atual
static int menu_context = 0; // 0: Menu principal, 1: Configuração de alarme, etc.
// Variável para rastrear a última opção desenhada
static int last_selected_option = -1;
// Variável para verificar se o display deve ser limpo
static bool clear_display = false;

const char *ringtone_options[] = {
    "Ringtone1",
    "Ringtone2",
    "Ringtone3"
};

#define NUM_RINGTONES (sizeof(ringtone_options) / sizeof(ringtone_options[0]))
// Variável global para armazenar a seleção atual do ringtone
static int selected_ringtone = 0; 

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
        oled_display_text("SEL A", 0, 50);

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
    printf("Configuring alarm...\n");
    int hours = alarm_hour;
    int minutes = alarm_minute;
    bool editing_hours = true; // Track what the user is editing
    oled_clear();

    oled_draw_line(0, 40, 120, 40);
    oled_display_text("SEL A", 0, 50);

    while (1) {
        char time_str[6];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);
        oled_display_text("Set Alarm:", 10, 5);
        oled_display_text(time_str, 30, 20);

        // Blinking underscore
        static bool show_underscore = true;
        if (show_underscore) {
            if (editing_hours) {
                oled_display_text("__", 30, 30);  // Underscore below hours
            } else {
                oled_display_text("__", 53, 30);  // Underscore below minutes
            }
        }
        show_underscore = !show_underscore; // Toggle blinking effect

        // Adjust time selection
        if (joystick_up()) {
            if (editing_hours) {
                hours = (hours + 1) % 24;
            } else {
                minutes = (minutes + 1) % 60;
            }
        } else if (joystick_down()) {
            if (editing_hours) {
                hours = (hours - 1 + 24) % 24;
            } else {
                minutes = (minutes - 1 + 60) % 60;
            }
        } else if (joystick_left()) {
            editing_hours = true;
            oled_display_text("__", 30, 30);  // Underscore below hours
            oled_display_text("  ", 53, 30);  // Underscore below minutes
        } else if (joystick_right()) {
            oled_display_text("  ", 30, 30);  // Underscore below hours
            oled_display_text("__", 53, 30);  // Underscore below minutes
            editing_hours = false;
        }

        // Confirm with Button A
        if (button_a_pressed()) {
            alarm_set = true;
            alarm_hour = hours;
            alarm_minute = minutes;
            sleep_ms(300);
            printf("Alarm set for %02d:%02d\n", alarm_hour, alarm_minute);
            menu_context = 0;
            oled_clear();
            oled_display_text("Alarm Set!", 10, 25);
            sleep_ms(1000);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        // Cancel with Button B
        if (button_b_pressed()) {
            printf("Exiting alarm setup.\n");
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        sleep_ms(300); // Controls blink speed and smooth navigation
    }
}


void check_alarm() {
    if (!alarm_set) return;

    datetime_t now;
    rtc_get_datetime(&now);

    if (alarm_set && now.hour == alarm_hour && now.min == alarm_minute) {
        printf("ALARM TRIGGERED at %02d:%02d!\n", now.hour, now.min);

        oled_clear();
        oled_display_text("ALARM!!!", 30, 20);
        oled_display_text("Press B to Stop", 10, 40);

        int blink_count = 0;

        while (1) {
            play_ringtone(selected_ringtone);  // 🔊 Play ringtone

            if (blink_count < 3) {  // 🔆 Blink LEDs three times using npSetLED()
                for (uint i = 0; i < LED_COUNT; i++) {
                    npSetLED(i, 100, 100, 100);  // Dim white light
                }
                matrix_update();
                sleep_ms(500);

                matrix_clear();
                matrix_update();
                sleep_ms(500);
                blink_count++;
            }

            // Allow user to stop alarm
            if (button_b_pressed()) {
                stop_buzzer(); // Stop ringtone
                printf("Alarm Stopped\n");
                oled_clear();
                oled_display_text("Alarm Stopped", 10, 20);
                sleep_ms(1000);
                oled_clear();
                draw_menu(-1);
                alarm_set = false; // Disable alarm
                clear_display = false;
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


/* void configure_lighting() {
    printf("Configuring Lighting...\n");
    oled_clear();
    oled_draw_line(0, 43, 120, 43);
    oled_display_text("SEL A", 0, 50);

    uint8_t selected_color = 0;
    const char* color_options[] = {"White", "Blue", "Red", "Yellow"};
    uint8_t color_values[][3] = {
        {255, 255, 255}, // White
        {0, 0, 255},     // Blue
        {255, 0, 0},     // Red
        {255, 255, 0}    // Yellow
    };

    // ✅ Set default color
    matrix_set_color(color_values[selected_color][0],
                     color_values[selected_color][1],
                     color_values[selected_color][2]);

    while (1) {
        oled_display_text("Adjust Lighting", 0, 0);

        // ✅ Show brightness level (0%-100%)
        char brightness_str[16];
        snprintf(brightness_str, sizeof(brightness_str), "Brightness: %d%%",
                 (brightness_index * 10));
        oled_display_text(brightness_str, 0, 20);

        // ✅ Show selected color
        char color_str[16];
        snprintf(color_str, sizeof(color_str), "Color: %s", color_options[selected_color]);
        oled_display_text(color_str, 0, 30);

        // ✅ Adjust brightness (Up/Down)
        if (joystick_up()) {
            matrix_adjust_brightness(1); // Increase brightness
        } else if (joystick_down()) {
            matrix_adjust_brightness(-1); // Decrease brightness
        }

        // ✅ Change color (Left/Right)
        if (joystick_right()) {
            selected_color = (selected_color + 1) % 4;
            matrix_set_color(color_values[selected_color][0],
                             color_values[selected_color][1],
                             color_values[selected_color][2]);
        } else if (joystick_left()) {
            selected_color = (selected_color - 1 + 4) % 4;
            matrix_set_color(color_values[selected_color][0],
                             color_values[selected_color][1],
                             color_values[selected_color][2]);
        }

        // Confirma config
        if (button_a_pressed()) {
            sleep_ms(300);
            printf("Lighting adjusted, Color: %s\n", color_options[selected_color]);
            menu_context = 0;
            oled_clear();
            oled_display_text("Lighting Set!", 10, 25);
            sleep_ms(1500);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        // Sai do menu
        if (button_b_pressed()) {
            printf("Exiting Lighting Settings.\n");
            matrix_off(); // Desliga LED
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        sleep_ms(200);
    }
} */



void reset_settings() {
    printf("Resetting settings...\n");
    oled_clear();

    int confirm_selection = 0;  // 0 = No, 1 = Yes

    while (1) {
        oled_display_text("Reset Settings?", 10, 5);
        oled_display_text("Yes", 30, 20);
        oled_display_text("No", 30, 30);

        oled_display_text(confirm_selection == 0 ? ">" : " ", 20, 20);
        oled_display_text(confirm_selection == 1 ? ">" : " ", 20, 30);

        if (joystick_down() || joystick_up()) {
            confirm_selection = !confirm_selection;
        }

        if (button_a_pressed()) {
            if (confirm_selection == 0) { // "Yes" selected
                alarm_hour = DEFAULT_ALARM_HOUR;
                alarm_minute = DEFAULT_ALARM_MINUTE;
                selected_ringtone = DEFAULT_RINGTONE;
                selected_color = DEFAULT_COLOR;
                alarm_set = false;

                printf("Settings reset to default!\n");
                oled_clear();
                oled_display_text("Settings Reset!", 10, 20);
                sleep_ms(1000);
                oled_clear();
            }
            clear_display = false;
            break;
        }

        if (button_b_pressed()) {
            printf("Reset canceled!\n");
            sleep_ms(300);
            oled_clear();   
            clear_display = false;
            break;
        }

        sleep_ms(200);
    }

    menu_context = 0;
    draw_menu(-1);
    clear_display = false;
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
                        printf("Entrando na configuração de ringtone\n");
                        menu_context = 2;
                        configure_ringtone();
                        break;
                    case 2:
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


