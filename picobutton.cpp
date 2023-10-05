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

#pragma pack(push, 1)
typedef struct
{
    /* frame */
    uint16_t size;
    uint16_t protocol : 12;
    uint8_t addressable : 1;
    uint8_t tagged : 1;
    uint8_t origin : 2;
    uint32_t source;
    /* frame address */
    uint8_t target[8];
    uint8_t reserved[6];
    uint8_t res_required : 1;
    uint8_t ack_required : 1;
    uint8_t : 6;
    uint8_t sequence;
    /* protocol header */
    uint64_t : 64;
    uint16_t type;
    uint16_t : 16;
    /* variable length payload follows */
} lx_protocol_header_t;
#pragma pack(pop)

// message using protocol header type
lx_protocol_header_t header = {
    .size = 0x2a00,
    .protocol = 0x0030,
    .addressable = 0b1,
    .tagged = 0b0,
    .origin = 0b00,
    .source = 0x843cf0,
    .target = {0x84, 0xf0, 0x3c, 0x84, 0x00, 0x00, 0x00, 0x00},
    .reserved = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .res_required = 0x00,
    .ack_required = 0x00,
    .sequence = 0x01,
    .type = 0x0d00,
};

const char payload[] = {
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
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(header) + sizeof(payload) + 1, PBUF_RAM);
        char *req = (char *)p->payload;

        memcpy(req, &header, sizeof(header));
        memcpy(req + sizeof(header), payload, sizeof(payload));

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