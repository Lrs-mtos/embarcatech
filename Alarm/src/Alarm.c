#include <stdio.h>
#include "pico/stdlib.h"
#include "oled.h"
#include "menu.h"
#include "joystick.h" 
#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include "rtc.h"
#include "buzzer.h"
#include "matrix.h"
#include "wifi_time.h"

#define BUZZER_PIN 21  // Define the pin for the buzzer

int main() {
    stdio_init_all();
    oled_init();
    oled_display_text("Initializing\n\n     Alarm", 12, 20);
    joystick_init();
    rtc_init_custom();
    buzzer_init();  // Initialize buzzer
    matrix_init();  // Initialize LED matrix
    wifi_time_init();  // Initialize WiFi and fetch time
    oled_clear();
    rtc_set_time(0, 0, 0, 0, 0, 0);
    
    while (1) {
        menu_navigation(); // Keeps the menu running
        check_alarm();     // Check for alarm
        update_time_display(); // Update time display
    }
}
