#ifndef RTC_H
#define RTC_H

#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

// Function prototypes
void rtc_init_custom();
void rtc_set_time(int year, int month, int day, int hour, int min, int sec);
void rtc_get_time(datetime_t *now);
bool rtc_check_alarm(int alarm_hour, int alarm_minute);

#endif
