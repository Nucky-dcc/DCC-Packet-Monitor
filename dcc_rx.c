#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "dcc_rx.h"

#define DCC_PIN 2

static DCC_EDGE rx_buffer[DCC_RX_BUFFER_SIZE];

static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static int last_state;
static uint32_t last_time;

void dcc_init(void)
{
    gpio_init(DCC_PIN);
    gpio_set_dir(DCC_PIN, GPIO_IN);

    memset(rx_buffer, 0, sizeof(rx_buffer));

    rx_head = 0;
    rx_tail = 0;

    last_state = gpio_get(DCC_PIN);
    last_time  = time_us_32();
}

void dcc_task(void)
{
    int state = gpio_get(DCC_PIN);

    if(state != last_state)
    {
        uint32_t now = time_us_32();
        uint32_t width = now - last_time;

        uint16_t next_head =
            (rx_head + 1) % DCC_RX_BUFFER_SIZE;

        if(next_head != rx_tail)
        {
            rx_buffer[rx_head].timestamp_us = now;
            rx_buffer[rx_head].width_us     = width;
            rx_buffer[rx_head].level        = (uint8_t)state;

            rx_head = next_head;
        }

        last_time  = now;
        last_state = state;
    }
}

bool dcc_get_edge(DCC_EDGE *edge)
{
    if(rx_head == rx_tail)
    {
        return false;
    }

    *edge = rx_buffer[rx_tail];

    rx_tail =
        (rx_tail + 1) % DCC_RX_BUFFER_SIZE;

    return true;
}