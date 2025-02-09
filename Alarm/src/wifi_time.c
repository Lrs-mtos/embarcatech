#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h" // for Wi-Fi
#include "hardware/rtc.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "wifi_time.h"
#include "rtc.h" // your custom rtc_set_time(year,month,day,hour,min,sec)

#define WIFI_SSID "brisa-805138"
#define WIFI_PASS "ysy7x2yi"

// NTP Servers (choose one):
#define NTP_SERVER    "pool.ntp.org"
// Alternatively: "time.google.com", "a.st1.ntp.br", etc.

// NTP operates on port 123 (UDP)
#define NTP_PORT      123
// 1900 to 1970 offset in seconds
#define NTP_TIMESTAMP_DELTA 2208988800ul

// For simplicity, let's do a blocking wait up to X ms for the NTP response
#define NTP_MAX_WAIT_MS  5000

// A small struct for controlling flow between send and receive
static volatile bool ntp_response_received = false;
static volatile uint32_t received_ntp_seconds = 0;

//////////////////////////////////////////////////////////////////
// A simplified NTP packet structure (48 bytes in total).
// We only care about the transmit timestamp for this example.
//////////////////////////////////////////////////////////////////
typedef struct {
    uint8_t li_vn_mode;      // Leap indicator (2 bits), Version (3 bits), Mode (3 bits)
    uint8_t stratum;
    uint8_t poll;
    int8_t  precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_id;
    uint32_t ref_timestamp[2];
    uint32_t orig_timestamp[2];
    uint32_t rx_timestamp[2];
    uint32_t tx_timestamp[2];
} ntp_packet_t;

// ----------------------------------------------------------------------------
// UDP receive callback
// ----------------------------------------------------------------------------
static void ntp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                        const ip_addr_t *addr, u16_t port) {
    // If we got a packet, parse it
    if (!p) {
        return;
    }

    if (p->len >= sizeof(ntp_packet_t)) {
        ntp_packet_t packet;
        pbuf_copy_partial(p, &packet, sizeof(ntp_packet_t), 0);

        // The transmit timestamp starts at byte 40 of the NTP packet
        // (that is the second half of packet.tx_timestamp[0..1]).
        // It's a 64-bit value, the first 32 bits are "seconds since 1900".
        uint32_t tx_time_seconds = ntohl(packet.tx_timestamp[0]); // seconds
        // uint32_t tx_time_fraction = ntohl(packet.tx_timestamp[1]); // fraction (not used here)

        // Convert NTP time to Unix time (seconds since 1970)
        uint32_t unix_time = tx_time_seconds - NTP_TIMESTAMP_DELTA;

        received_ntp_seconds = unix_time;
        ntp_response_received = true;
    }

    // Always free the buffer
    pbuf_free(p);
}

// ----------------------------------------------------------------------------
// Helper to do a blocking NTP request
// ----------------------------------------------------------------------------
static bool get_ntp_time_manually(void) {
    // Step 1: Create UDP PCB
    struct udp_pcb *ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!ntp_pcb) {
        printf("Failed to create UDP pcb\n");
        return false;
    }

    // Step 2: Bind to any local port
    if (udp_bind(ntp_pcb, IP_ANY_TYPE, 0) != ERR_OK) {
        printf("Failed to bind UDP pcb\n");
        udp_remove(ntp_pcb);
        return false;
    }

    // Step 3: Set up receive callback
    udp_recv(ntp_pcb, ntp_recv_cb, NULL);

    // Step 4: Resolve the NTP server (DNS). We do a blocking DNS lookup for simplicity.
    // If you prefer non-blocking, you'd use dns_gethostbyname with a callback, etc.
    ip_addr_t ntp_server_ip;
    err_t err = dns_gethostbyname(NTP_SERVER, &ntp_server_ip, NULL, NULL);
    if (err == ERR_INPROGRESS) {
        // blocking wait for DNS resolution is a bit tricky in raw lwIP,
        // but for simplicity, let's just poll until the IP resolves or timeout:
        absolute_time_t dns_timeout = make_timeout_time_ms(3000);
        while (time_reached(dns_timeout) == false) {
            // attempt again
            err = dns_gethostbyname(NTP_SERVER, &ntp_server_ip, NULL, NULL);
            if (err != ERR_INPROGRESS && err != ERR_OK) {
                break;
            }
            if (err == ERR_OK) {
                break;
            }
            cyw43_arch_poll();
            sleep_ms(50);
        }
    }
    if (err == ERR_OK) {
        printf("NTP server IP resolved: %s\n", ipaddr_ntoa(&ntp_server_ip));
    } else {
        printf("DNS lookup failed for %s\n", NTP_SERVER);
        udp_remove(ntp_pcb);
        return false;
    }

    // Step 5: Prepare NTP request packet (48 bytes)
    ntp_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    // LI = 0 (no leap), VN = 4 (version), Mode = 3 (client)
    //  (LI << 6) | (VN << 3) | Mode => (0 << 6) | (4 << 3) | 3 = 0x1B
    packet.li_vn_mode = 0x1B;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ntp_packet_t), PBUF_RAM);
    if (!p) {
        printf("Failed to allocate pbuf for NTP request\n");
        udp_remove(ntp_pcb);
        return false;
    }
    pbuf_take(p, &packet, sizeof(packet));

    // Step 6: Send the request to the NTP server
    err_t send_err = udp_sendto(ntp_pcb, p, &ntp_server_ip, NTP_PORT);
    pbuf_free(p);
    if (send_err != ERR_OK) {
        printf("Failed to send NTP request\n");
        udp_remove(ntp_pcb);
        return false;
    }
    printf("NTP request sent. Waiting for reply...\n");

    // Step 7: Block until we get a response or timeout
    ntp_response_received = false;
    received_ntp_seconds  = 0;
    absolute_time_t timeout = make_timeout_time_ms(NTP_MAX_WAIT_MS);

    while (!time_reached(timeout)) {
        if (ntp_response_received) {
            break;
        }
        // We must let LWIP process incoming packets
        cyw43_arch_poll();
        sleep_ms(10);
    }

    // Step 8: Clean up
    udp_remove(ntp_pcb);

    // Check if we got the time
    if (!ntp_response_received) {
        printf("NTP response not received (timeout)\n");
        return false;
    }

    // If we got here, we have `received_ntp_seconds`
    // Convert into a struct tm and set RTC
    // (Note: We do a localtime or gmtime as needed.)
    time_t t = (time_t) received_ntp_seconds;
    t -= 3 * 3600;
    struct tm *tm_info = gmtime(&t); // or localtime(&t)

    // Suppose your "rtc_set_time()" expects (year, mon, day, hr, min, sec)
    // tm_year is years since 1900, tm_mon is 0..11
    rtc_set_time(
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        tm_info->tm_hour,
        tm_info->tm_min,
        tm_info->tm_sec
    );

    printf("Time set via manual NTP: %04d-%02d-%02d %02d:%02d:%02d (UTC)\n",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    return true;
}

// ----------------------------------------------------------------------------
// Public function from wifi_time.h
// ----------------------------------------------------------------------------
void wifi_time_init(void) {
    // 1) Initialize Wi-Fi (pico_cyw43_arch)
    //    e.g. for "poll" or "threadsafe background" builds:
    if (cyw43_arch_init()) {
        printf("Failed to init cyw43_arch\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    // 2) Connect to the AP
    printf("Connecting to Wi-Fi: %s\n", WIFI_SSID);
    int connect_status = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000);
    if (connect_status) {
        printf("Failed to connect. Status=%d\n", connect_status);
        // You could call cyw43_arch_deinit() if you want to power down
        return;
    }
    // Print IP
    uint8_t *ip = (uint8_t*) &cyw43_state.netif[0].ip_addr.addr;
    printf("Connected! IP = %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

    // 3) Manually fetch time from an NTP server
    if (!get_ntp_time_manually()) {
        printf("NTP sync failed.\n");
    }
    else {
        printf("NTP sync success!\n");
    }

    // If you don't need Wi-Fi anymore, you could call
    // cyw43_arch_deinit();
}
