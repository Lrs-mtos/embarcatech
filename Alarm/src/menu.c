#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"      // For sleep_ms, stdio_init_all, etc.
#include "oled.h"             // For OLED display functions
#include "joystick.h"         // For joystick navigation
#include "menu.h"             // (Assumed to have menu declarations)
#include "hardware/rtc.h"     // For RTC access
#include "pico/util/datetime.h" // For datetime_t and rtc_get_datetime
#include "buzzer.h"           // For buzzer control
#include "matrix.h"           // For LED matrix control

// -------------------------------------------------------------------------
// Default settings and constants
// -------------------------------------------------------------------------
#define BUZZER_PIN 21             

#define DEFAULT_ALARM_HOUR 12
#define DEFAULT_ALARM_MINUTE 0
#define DEFAULT_RINGTONE 0
#define DEFAULT_COLOR 0
#define DEFAULT_BRIGHTNESS 10

// -------------------------------------------------------------------------
// Global variables to store user settings
// -------------------------------------------------------------------------
static int alarm_hour = DEFAULT_ALARM_HOUR;
static int alarm_minute = DEFAULT_ALARM_MINUTE;
static int alarm_second = 0;
static int ringtone = DEFAULT_RINGTONE;
static int color = DEFAULT_COLOR;
static int brightness = DEFAULT_BRIGHTNESS;

// Menu options strings
const char *menu_options[] = {
    "1 Alarm",
    "2 Ringtone",
    "3 Reset"
};
#define NUM_OPTIONS (sizeof(menu_options) / sizeof(menu_options[0]))

// Global variables for menu context and display state
static int menu_context = 0;               // 0: Main menu, 1: Alarm config, etc.
static int last_selected_option = -1;      // Tracks last drawn selection index
static bool clear_display = false;         // Flag for forcing a display clear

// Ringtone options
const char *ringtone_options[] = {
    "1 Simple",
    "2 Tones",
    "3 Star"
};
#define NUM_RINGTONES (sizeof(ringtone_options) / sizeof(ringtone_options[0]))
static int selected_ringtone = 0;          // Currently selected ringtone

static int selected_color = 0;             // Currently selected color (unused in active code)

// Alarm state
static bool alarm_set = false;             // true if alarm is active

// -------------------------------------------------------------------------
// SECTION: MENU DISPLAY FUNCTIONS
// -------------------------------------------------------------------------

/*
 * draw_menu: Draws the main menu on the OLED display.
 *   - selected_option: the index of the currently highlighted option.
 *
 * The function also displays the current RTC time (fetched from the RTC)
 * and indicates whether the alarm is set.
 */
void draw_menu(int selected_option) {
    // Only redraw if the selected option has changed
    if (selected_option != last_selected_option) {
        // Clear the display if needed when changing selection
        if (clear_display) {
            oled_clear();
        }
        clear_display = true;

        // Fetch current RTC time
        datetime_t now;
        rtc_get_datetime(&now);

        // Format the RTC time as HH:MM:SS
        char current_time[10];
        snprintf(current_time, sizeof(current_time), "%02d:%02d:%02d", now.hour, now.min, now.sec);

        // Draw each menu option with a selection arrow for the highlighted option
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected_option) {
                oled_display_text(">", 0, i * 10); // Show selection arrow
            }
            oled_display_text(menu_options[i], 10, i * 10);
            // Draw current time in a fixed position on the screen
            oled_display_text(current_time, 50, 50);
        }

        // Indicate alarm state: display a checkmark if alarm is set
        if (alarm_set) {
            oled_display_text("(V)", 73, 0);
        } else {
            oled_display_text("   ", 73, 0);
        }

        // Draw a separator line and label at the bottom
        oled_draw_line(0, 40, 120, 40);
        oled_display_text("SEL A", 0, 50);

        // Remember this option as the last one drawn
        last_selected_option = selected_option;
    }
}

/*
 * update_time_display: Updates only the time display portion of the menu.
 *
 * This function fetches the current RTC time and updates the portion
 * of the OLED where the time is shown without redrawing the entire menu.
 */
void update_time_display() {
    datetime_t now;
    rtc_get_datetime(&now);

    char current_time[10];
    snprintf(current_time, sizeof(current_time), "%02d:%02d:%02d", now.hour, now.min, now.sec);
    oled_display_text(current_time, 50, 50);
}

// -------------------------------------------------------------------------
// SECTION: ALARM CONFIGURATION FUNCTIONS
// -------------------------------------------------------------------------

/*
 * configure_alarm: Allows the user to set the alarm time.
 *
 * The user can select hours and minutes using the joystick, with a blinking
 * underscore indicating the current editing field. The alarm is confirmed with
 * Button A and canceled with Button B.
 */
void configure_alarm() {
    printf("Configuring alarm...\n");

    // Initialize temporary values for hours and minutes
    int hours = alarm_hour;
    int minutes = alarm_minute;
    bool editing_hours = true;  // Start by editing hours

    oled_clear();
    oled_draw_line(0, 40, 120, 40);
    oled_display_text("SEL A", 0, 50);

    while (1) {
        char time_str[6];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);
        oled_display_text("Set Alarm:", 10, 5);
        oled_display_text(time_str, 30, 20);

        // Blinking underscore to indicate active editing field
        static bool show_underscore = true;
        if (show_underscore) {
            if (editing_hours) {
                oled_display_text("__", 30, 30);  // Underscore below hours
            } else {
                oled_display_text("__", 53, 30);  // Underscore below minutes
            }
        }
        show_underscore = !show_underscore; // Toggle blinking

        // Use joystick to adjust hours or minutes
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
            // Switch to editing hours
            editing_hours = true;
            oled_display_text("__", 30, 30);  // Show underscore under hours
            oled_display_text("  ", 53, 30);   // Clear underscore under minutes
        } else if (joystick_right()) {
            // Switch to editing minutes
            oled_display_text("  ", 30, 30);   // Clear underscore under hours
            oled_display_text("__", 53, 30);    // Show underscore under minutes
            editing_hours = false;
        }

        // Confirm with Button A: set alarm
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

        // Cancel with Button B: return to main menu
        if (button_b_pressed()) {
            printf("Exiting alarm setup.\n");
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }
        sleep_ms(300);  // Delay for blinking and smooth navigation
    }
}

/*
 * check_alarm: Checks if the alarm time matches the current RTC time.
 *
 * When the alarm is triggered, it plays a ringtone, blinks the LED matrix,
 * and allows the user to stop the alarm using Button B.
 */
void check_alarm() {
    if (!alarm_set) return;

    datetime_t now;
    rtc_get_datetime(&now);

    // Trigger the alarm exactly when hour, minute match and second is zero.
    if (alarm_set && now.hour == alarm_hour && now.min == alarm_minute && now.sec == 0) {
        printf("ALARM TRIGGERED at %02d:%02d!\n", now.hour, now.min);

        oled_clear();
        oled_display_text("ALARM!!!", 30, 20);
        oled_display_text("Sel B to Stop", 10, 40);

        int blink_count = 0;
        while (1) {
            play_ringtone(selected_ringtone);  // Play ringtone sound

            // Blink LED matrix three times
            if (blink_count < 3) {
                for (uint i = 0; i < LED_COUNT; i++) {
                    npSetLED(i, 100, 100, 100);  // Set a dim white light
                }
                matrix_update();
                sleep_ms(500);
                matrix_clear();
                matrix_update();
                sleep_ms(500);
                blink_count++;
            }

            // Stop alarm when Button B is pressed
            if (button_b_pressed()) {
                stop_buzzer();  // Stop playing ringtone
                printf("Alarm Stopped\n");
                oled_clear();
                oled_display_text("Alarm Stopped", 10, 20);
                sleep_ms(1000);
                oled_clear();
                draw_menu(-1);
                alarm_set = false;  // Reset alarm flag
                clear_display = false;
                break;
            }
        }
    }
}

// -------------------------------------------------------------------------
// SECTION: RINGTONE CONFIGURATION FUNCTIONS
// -------------------------------------------------------------------------

/*
 * configure_ringtone: Allows the user to select a ringtone from a list.
 *
 * Navigation is done via the joystick and selection is confirmed with Button A.
 * Button B cancels and returns to the main menu.
 */
void configure_ringtone() {
    printf("Configurando o ringtone...\n");
    oled_clear();

    while (1) {
        oled_display_text("Select Ringtone:", 0, 0);

        // Display ringtone options with selection indicator
        for (int i = 0; i < NUM_RINGTONES; i++) {
            if (i == selected_ringtone) {
                oled_display_text(">", 0, (i * 10) + 10); // Selection arrow
            }
            oled_display_text(ringtone_options[i], 10, (i * 10) + 10);
        }

        // Navigate through options with joystick
        if (joystick_down()) {
            selected_ringtone = (selected_ringtone + 1) % NUM_RINGTONES;
            oled_clear();
        } else if (joystick_up()) {
            selected_ringtone = (selected_ringtone - 1 + NUM_RINGTONES) % NUM_RINGTONES;
            oled_clear();
        }

        // Confirm selection with Button A
        if (button_a_pressed()) {
            sleep_ms(300);
            printf("Ringtone selecionado: %s\n", ringtone_options[selected_ringtone]);
            menu_context = 0; // Return to main menu
            oled_clear();
            oled_display_text("Ringtone\nSelected:", 0, 20);
            oled_display_text(ringtone_options[selected_ringtone], 0, 40);
            sleep_ms(1000);
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        // Cancel with Button B, returning to main menu
        if (button_b_pressed()) {
            printf("Voltando ao menu principal sem alterar o ringtone.\n");
            menu_context = 0;
            oled_clear();
            draw_menu(-1);
            clear_display = false;
            break;
        }

        sleep_ms(200); // Delay for smooth navigation
    }
}

// -------------------------------------------------------------------------
// SECTION: SETTINGS RESET FUNCTION
// -------------------------------------------------------------------------

/*
 * reset_settings: Resets the alarm, ringtone, and color settings to defaults.
 *
 * Displays a confirmation menu ("Yes" or "No") and resets settings if "Yes"
 * is selected. Returns to the main menu afterward.
 */
void reset_settings() {
    printf("Resetting settings...\n");
    oled_clear();

    int confirm_selection = 0;  // 0 = Yes, 1 = No

    while (1) {
        oled_display_text("Reset Settings?", 10, 5);
        oled_display_text("Yes", 30, 20);
        oled_display_text("No", 30, 30);

        // Display selection arrow for the current confirmation choice
        oled_display_text(confirm_selection == 0 ? ">" : " ", 20, 20);
        oled_display_text(confirm_selection == 1 ? ">" : " ", 20, 30);

        // Toggle selection with joystick up/down
        if (joystick_down() || joystick_up()) {
            confirm_selection = !confirm_selection;
        }

        // Confirm selection with Button A
        if (button_a_pressed()) {
            if (confirm_selection == 0) { // "Yes" selected: reset defaults
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
                clear_display = false;
                break;
            } else {
                clear_display = true;
                break;
            }
        }

        // Cancel reset with Button B
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

// -------------------------------------------------------------------------
// SECTION: MENU NAVIGATION
// -------------------------------------------------------------------------

/*
 * menu_navigation: Main entry point for menu interaction.
 *
 * In the main menu (menu_context == 0), the user navigates through the menu
 * options using the joystick. Pressing Button A selects an option, triggering
 * the corresponding configuration function.
 */
void menu_navigation() {
    static int selected_option = 0;
    if (menu_context == 0) { // Main menu
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
                case 0: // Alarm configuration
                    printf("Entrando na configuração do alarme\n");
                    menu_context = 1;
                    configure_alarm();
                    break;
                case 1: // Ringtone configuration
                    printf("Entrando na configuração de ringtone\n");
                    menu_context = 2;
                    configure_ringtone();
                    break;
                case 2: // Reset settings
                    printf("Redefinindo configurações\n");
                    menu_context = 2;
                    reset_settings();
                    break;
            }
        }
        // Button B is ignored in the main menu
        if (button_b_pressed()) {
            printf("Botão B ignorado no menu principal\n");
        }
    }
}
