#include "buzzer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

#define BUZZER_PIN 21  // Define the pin for the buzzer

// Ringtones
static const uint ringtone1[] = {262, 294, 330, 349, 392, 440, 494, 523}; // Simple scale
static const uint ringtone2[] = {659, 698, 784, 880, 988}; // Higher tones
static const uint ringtone3[] = {330, 330, 330, 262, 392, 523, 330}; // Short Star Wars melody

// Note durations (uniform for all ringtones)
static const uint note_durations[] = {200, 200, 200, 200, 200, 200, 200, 200};

// Initialize PWM for the buzzer
void buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Play a single tone
void play_tone(uint frequency, uint duration_ms) {
    if (frequency == 0) {
        return; // Avoid division by zero if an invalid frequency is passed
    }

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = (clock_freq / frequency) - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(BUZZER_PIN, top / 2); // 50% duty cycle

    sleep_ms(duration_ms);

    pwm_set_gpio_level(BUZZER_PIN, 0); // Turn off buzzer
    sleep_ms(50); // Short pause between notes
}

// Play a selected ringtone
void play_ringtone(int ringtone_option) {
    const uint *notes = NULL;
    int length = 0;

    switch (ringtone_option) {
        case 0:
            notes = ringtone1;
            length = sizeof(ringtone1) / sizeof(ringtone1[0]);
            break;
        case 1:
            notes = ringtone2;
            length = sizeof(ringtone2) / sizeof(ringtone2[0]);
            break;
        case 2:
            notes = ringtone3;
            length = sizeof(ringtone3) / sizeof(ringtone3[0]);
            break;
        default:
            printf("Invalid ringtone option: %d\n", ringtone_option);
            return; // Exit if invalid option
    }

    for (int i = 0; i < length; i++) {
        play_tone(notes[i], note_durations[i % (sizeof(note_durations) / sizeof(note_durations[0]))]);
    }
}

// Stop the buzzer
void stop_buzzer() {
    pwm_set_gpio_level(BUZZER_PIN, 0);
}
