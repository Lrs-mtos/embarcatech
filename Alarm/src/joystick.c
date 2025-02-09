#include "joystick.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include <stdio.h>

// Joystick ADC pins
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26

// Threshold values for movement detection
#define JOYSTICK_THRESHOLD_LEFT 500
#define JOYSTICK_THRESHOLD_RIGHT 3900
#define JOYSTICK_THRESHOLD_UP 3900
#define JOYSTICK_THRESHOLD_DOWN 500

// Button pins
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Debounce time in milliseconds
#define DEBOUNCE_TIME_MS 250

// Button press flags
static volatile bool button_a_flag = false;
static volatile bool button_b_flag = false;

// Last press timestamps (milliseconds)
static volatile uint32_t last_press_a = 0;
static volatile uint32_t last_press_b = 0;

// Interrupt handler for button presses
static void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = time_us_32() / 1000; // Get current time in ms

    if (gpio == BUTTON_A_PIN && (now - last_press_a > DEBOUNCE_TIME_MS)) {
        last_press_a = now;
        button_a_flag = true;
    }

    if (gpio == BUTTON_B_PIN && (now - last_press_b > DEBOUNCE_TIME_MS)) {
        last_press_b = now;
        button_b_flag = true;
    }
}

// Initialize joystick and buttons
void joystick_init(void) {
    sleep_ms(500); // Small delay to stabilize initialization
    
    // Initialize ADC for joystick axes
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Initialize button A
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Initialize button B
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

// Read joystick movement
bool joystick_left(void) {
    adc_select_input(1);
    return adc_read() < JOYSTICK_THRESHOLD_LEFT;
}

bool joystick_right(void) {
    adc_select_input(1);
    return adc_read() > JOYSTICK_THRESHOLD_RIGHT;
}

bool joystick_up(void) {
    adc_select_input(0);
    return adc_read() > JOYSTICK_THRESHOLD_UP;
}

bool joystick_down(void) {
    adc_select_input(0);
    return adc_read() < JOYSTICK_THRESHOLD_DOWN;
}

// Read button states
bool button_a_pressed(void) {
    if (button_a_flag) {
        button_a_flag = false;
        return true;
    }
    return false;
}

bool button_b_pressed(void) {
    if (button_b_flag) {
        button_b_flag = false;
        return true;
    }
    return false;
}
