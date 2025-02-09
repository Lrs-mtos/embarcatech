#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"

// Buzzer pin definition
#define BUZZER_PIN 21

// Function prototypes
void buzzer_init();
void play_tone(uint frequency, uint duration_ms);
void play_ringtone(int ringtone_option);
void stop_buzzer();

#endif
