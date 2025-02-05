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

#define BUZZER_PIN 21  // Define the pin for the buzzer

int main() {
    stdio_init_all();
    oled_init();
    joystick_init();
    rtc_init_custom();
    buzzer_init();  //Initialize buzzer
    matrix_init();  // Initialize LED matrix

    

    // Set initial RTC time
    rtc_set_time(2024, 2, 5, 12, 0, 0);

    while (1) {
        menu_navigation(); // Keeps the menu running
        check_alarm();     // Check for alarm
        update_time_display(); // Update time display
        //sleep_ms(50); // Avoid CPU overload
    }
}