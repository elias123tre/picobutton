#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 56700
#define BEACON_TARGET "192.168.1.11"
#define BEACON_INTERVAL_MS 30000

void onboard_led(bool on)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

void error_lockup(const char *message)
{
    while (true)
    {
        sleep_ms(1000);
        printf(message);
    }
}

char message[] = {
    0x2a,
    0x00,
    0x00,
    0x34,
    0xb4,
    0x3c,
    0xf0,
    0x84,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x01,
    0x0d,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x75,
    0x00,
    0x00,
    0x00,
    0xff,
    0xff,
    0xe8,
    0x03,
    0x00,
    0x00,
};

void run_udp_beacon()
{
    struct udp_pcb *pcb = udp_new();

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 0;
    while (true)
    {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(message) + 1, PBUF_RAM);
        char *req = (char *)p->payload;
        memmove(req, message, sizeof(message));
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p);
        if (er != ERR_OK)
        {
            printf("Failed to send UDP packet! error=%d", er);
        }
        else
        {
            printf("Sent packet %d\n", counter);
            counter++;
        }

        // Note in practice for this simple UDP transmitter,
        // the end result for both background and poll is the same

        sleep_ms(BEACON_INTERVAL_MS);
    }
}

int main()
{
    stdio_init_all();

    // initialise the wifi chip and onboard led
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN))
    {
        error_lockup("failed to init wifi\n");
        return 1;
    }
    printf("initialised\n");

    onboard_led(true);
    sleep_ms(250);
    onboard_led(false);
    sleep_ms(250);
    onboard_led(true);

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        error_lockup("failed to connect\n");
        return 1;
    }
    printf("Connected.\n");

    run_udp_beacon();

    while (true)
    {
        onboard_led(true);
        sleep_ms(1000);
        printf("Blinked!\n");
    }

    cyw43_arch_deinit();
    return 0;
}