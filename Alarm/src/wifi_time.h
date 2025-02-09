#ifndef WIFI_TIME_H
#define WIFI_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Wi-Fi, connect to the AP, fetch NTP time manually, and update RTC.
 */
void wifi_time_init(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_TIME_H
