#include "rtc.h"
#include <stdio.h>

void rtc_init_custom() {
    rtc_init(); // Initialize RTC hardware
}

void rtc_set_time(int year, int month, int day, int hour, int min, int sec) {

    if (!rtc_running()) {
        printf("RTC is not running! Initializing...\n");
        rtc_init();
    }
    
    datetime_t t = {
        .year = year,
        .month = month,
        .day = day,
        .dotw = 1,
        .hour = hour,
        .min = min,
        .sec = sec
    };
    rtc_set_datetime(&t);
    printf("RTC Set");
}


void rtc_get_time(datetime_t *now) {
    rtc_get_datetime(now);
}

bool rtc_check_alarm(int alarm_hour, int alarm_minute) {
    datetime_t now;
    rtc_get_datetime(&now);

    if (now.hour == alarm_hour && now.min == alarm_minute) {
        printf("ALARM TRIGGERED: %02d:%02d\n", now.hour, now.min);
        return true;
    }
    return false;
}
