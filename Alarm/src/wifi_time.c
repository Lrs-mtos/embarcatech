#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/rtc.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "wifi_time.h"
#include "rtc.h"

#define WIFI_SSID "NOME_DA_REDE_WIFI"
#define WIFI_PASS "SENHA_DA_REDE_WIFI"

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_TIMESTAMP_DELTA 2208988800ul
#define NTP_MAX_WAIT_MS 5000

static volatile bool ntp_response_received = false;
static volatile uint32_t received_ntp_seconds = 0;

typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    int8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_id;
    uint32_t ref_timestamp[2];
    uint32_t orig_timestamp[2];
    uint32_t rx_timestamp[2];
    uint32_t tx_timestamp[2];
} ntp_packet_t;

static void ntp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                        const ip_addr_t *addr, u16_t port) {
    if (!p) return;

    if (p->len >= sizeof(ntp_packet_t)) {
        ntp_packet_t packet;
        pbuf_copy_partial(p, &packet, sizeof(ntp_packet_t), 0);
        uint32_t tx_time_seconds = ntohl(packet.tx_timestamp[0]);
        received_ntp_seconds = tx_time_seconds - NTP_TIMESTAMP_DELTA;
        ntp_response_received = true;
    }
    pbuf_free(p);
}

static bool get_ntp_time_manually(void) {
    struct udp_pcb *ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!ntp_pcb) {
        printf("Failed to create UDP PCB\n");
        return false;
    }

    if (udp_bind(ntp_pcb, IP_ANY_TYPE, 0) != ERR_OK) {
        printf("Failed to bind UDP PCB\n");
        udp_remove(ntp_pcb);
        return false;
    }

    udp_recv(ntp_pcb, ntp_recv_cb, NULL);

    ip_addr_t ntp_server_ip;
    err_t err = dns_gethostbyname(NTP_SERVER, &ntp_server_ip, NULL, NULL);
    if (err == ERR_INPROGRESS) {
        absolute_time_t dns_timeout = make_timeout_time_ms(3000);
        while (!time_reached(dns_timeout)) {
            err = dns_gethostbyname(NTP_SERVER, &ntp_server_ip, NULL, NULL);
            if (err == ERR_OK || err != ERR_INPROGRESS) break;
            cyw43_arch_poll();
            sleep_ms(50);
        }
    }

    if (err != ERR_OK) {
        printf("DNS lookup failed for %s\n", NTP_SERVER);
        udp_remove(ntp_pcb);
        return false;
    }

    ntp_packet_t packet = {0};
    packet.li_vn_mode = 0x1B;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ntp_packet_t), PBUF_RAM);
    if (!p) {
        printf("Failed to allocate pbuf for NTP request\n");
        udp_remove(ntp_pcb);
        return false;
    }

    pbuf_take(p, &packet, sizeof(packet));
    err_t send_err = udp_sendto(ntp_pcb, p, &ntp_server_ip, NTP_PORT);
    pbuf_free(p);
    if (send_err != ERR_OK) {
        printf("Failed to send NTP request\n");
        udp_remove(ntp_pcb);
        return false;
    }

    ntp_response_received = false;
    received_ntp_seconds = 0;
    absolute_time_t timeout = make_timeout_time_ms(NTP_MAX_WAIT_MS);

    while (!time_reached(timeout)) {
        if (ntp_response_received) break;
        cyw43_arch_poll();
        sleep_ms(10);
    }

    udp_remove(ntp_pcb);

    if (!ntp_response_received) {
        printf("NTP response not received (timeout)\n");
        return false;
    }

    time_t t = (time_t)received_ntp_seconds;
    t -= 3 * 3600;
    struct tm *tm_info = gmtime(&t);

    rtc_set_time(
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        tm_info->tm_hour,
        tm_info->tm_min,
        tm_info->tm_sec
    );

    printf("Time set via NTP: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    return true;
}

void wifi_time_init(void) {
    if (cyw43_arch_init()) {
        printf("Failed to initialize Wi-Fi\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi: %s\n", WIFI_SSID);
    int connect_status = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000);

    if (connect_status) {
        printf("Failed to connect. Status=%d\n", connect_status);
        return;
    }

    uint8_t *ip = (uint8_t*)&cyw43_state.netif[0].ip_addr.addr;
    printf("Connected! IP = %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

    if (!get_ntp_time_manually()) {
        printf("NTP sync failed.\n");
    } else {
        printf("NTP sync successful.\n");
    }
}
