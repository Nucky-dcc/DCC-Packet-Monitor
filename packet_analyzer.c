#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "packet_analyzer.h"

#include "loco_analyzer.h"
#include "accessory_analyzer.h"
#include "service_analyzer.h"

#define HEX_DISPLAY_WIDTH 6

static uint32_t packet_no;
static uint32_t last_timestamp;

void packet_analyzer_init(void)
{
    packet_no = 1;
    last_timestamp = 0;
}

static bool check_crc(
    const DCC_PACKET *packet)
{
    uint8_t crc = 0;

    if(packet->length < 2)
    {
        return false;
    }

    for(uint8_t i = 0;
        i < (packet->length - 1);
        i++)
    {
        crc ^= packet->data[i];
    }

    return
        (crc ==
         packet->data[
             packet->length - 1]);
}

void packet_analyzer(
    const DCC_PACKET *packet)
{
    uint32_t delta_us;
    uint32_t delta_ms;
    uint32_t delta_frac;

    if(packet == NULL)
    {
        return;
    }

    if(last_timestamp == 0)
    {
        delta_us = 0;
    }
    else
    {
        delta_us =
            packet->timestamp_us
            - last_timestamp;
    }

    last_timestamp =
        packet->timestamp_us;

    delta_ms   = delta_us / 1000;
    delta_frac = delta_us % 1000;

    printf("%06lu  %03lu.%03lums  ",
           (unsigned long)packet_no,
           (unsigned long)delta_ms,
           (unsigned long)delta_frac);

    packet_no++;

    if(packet_no > 999999)
    {
        packet_no = 1;
    }

    for(uint8_t i = 0;
        i < HEX_DISPLAY_WIDTH;
        i++)
    {
        if(i < packet->length)
        {
            printf("%02X ",
                   packet->data[i]);
        }
        else
        {
            printf("   ");
        }
    }

    printf("| ");

    if(!check_crc(packet))
    {
        printf("CRC ERROR\r\n");
        return;
    }

    if(packet->length == 3 &&
       packet->data[0] == 0xFF &&
       packet->data[1] == 0x00 &&
       packet->data[2] == 0xFF)
    {
        printf("Idle");
    }
    else if(packet->data[0] == 0x00)
    {
        printf("Reset");
    }
    else if(is_service_packet(packet))
    {
        analyze_service(packet);
    }
    else if((packet->data[0] & 0xC0) == 0x80)
    {
        analyze_accessory(packet);
    }
    else
    {
        analyze_loco(packet);
    }

    printf("\r\n");
}