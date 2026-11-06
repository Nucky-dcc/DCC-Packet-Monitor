#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "packet_analyzer.h"

#include "loco_analyzer.h"
#include "accessory_analyzer.h"
#include "service_analyzer.h"
#include "ops_analyzer.h"
#include "monitor_config.h"
#include "statistics.h"

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

static uint16_t get_packet_address(
    const DCC_PACKET *packet)
{
    if(packet == NULL)
    {
        return 0;
    }

    /*
     * Accessory
     */
    if((packet->data[0] & 0xC0) == 0x80)
    {
        uint16_t addr_9bit;
        uint8_t pair;

        addr_9bit =
            ((uint16_t)
             ((~packet->data[1]) & 0x70)
             << 2)
            |
            (packet->data[0] & 0x3F);

        pair =
            (packet->data[1] >> 1) & 0x03;

        return
            (addr_9bit > 0)
            ? ((addr_9bit - 1) * 4)
                + pair + 1
            : pair + 1;
    }

    /*
     * Long Address
     */
    if((packet->data[0] & 0xC0) == 0xC0)
    {
        return
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             |
             packet->data[1];
    }

    /*
     * Short Address
     */
    return packet->data[0];
}

void packet_analyzer(
    const DCC_PACKET *packet)
{
    uint32_t delta_us;
    uint32_t delta_ms;
    uint32_t delta_frac;
    uint32_t display_no;

    if(packet == NULL)
    {
        return;
    }

    statistics_count_total();

    bool show_packet = true;

    /*
    * RESET表示制御
    */
    if(packet->length == 3 &&
    packet->data[0] == 0x00 &&
    packet->data[1] == 0x00 &&
    packet->data[2] == 0x00)
    {
        if(!monitor_config.show_reset)
        {
            show_packet = false;
        }
    }

    /*
    * IDLE表示制御
    */
    if(packet->length == 3 &&
    packet->data[0] == 0xFF &&
    packet->data[1] == 0x00 &&
    packet->data[2] == 0xFF)
    {
        if(!monitor_config.show_idle)
        {
            show_packet = false;
        }
    }

    /*
    * 種別フィルタ
    */
    switch(monitor_config.filter)
    {
        case FILTER_ALL:
            break;

        case FILTER_LOCO:

            if(packet->data[0] == 0x00)
            {
                show_packet = false;
            }
            else if(packet->length == 3 &&
                    packet->data[0] == 0xFF &&
                    packet->data[1] == 0x00 &&
                    packet->data[2] == 0xFF)
            {
                show_packet = false;
            }
            else if(is_service_packet(packet))
            {
                show_packet = false;
            }
            else if(is_ops_packet(packet))
            {
                show_packet = false;
            }
            else if((packet->data[0] & 0xC0) == 0x80)
            {
                show_packet = false;
            }

            break;

        case FILTER_ACCESSORY:

            if(packet->data[0] == 0x00)
            {
                show_packet = false;
            }
            else if(packet->length == 3 &&
                    packet->data[0] == 0xFF &&
                    packet->data[1] == 0x00 &&
                    packet->data[2] == 0xFF)
            {
                show_packet = false;
            }
            else if(is_service_packet(packet))
            {
                show_packet = false;
            }
            else if(is_ops_packet(packet))
            {
                show_packet = false;
            }
            else if((packet->data[0] & 0xC0) != 0x80)
            {
                show_packet = false;
            }

            break;

        case FILTER_OPS:

            if(!is_ops_packet(packet))
            {
                show_packet = false;
            }

            break;

        default:
            break;
    }

    /*
    * アドレスフィルタ
    */
    if(monitor_config.address_filter != 0)
    {
        uint16_t address;

        address =
            get_packet_address(packet);

        if(address !=
        monitor_config.address_filter)
        {
            show_packet = false;
        }
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

    statistics_add_elapsed_us(
        delta_us);

    display_no = packet_no;

    packet_no++;

    if(packet_no > 999999)
    {
        packet_no = 1;
    }

    if(!show_packet)
    {
        return;
    }

    delta_ms   = delta_us / 1000;
    delta_frac = delta_us % 1000;

    printf("%06lu  %03lu.%03lums  ",
        (unsigned long)display_no,
        (unsigned long)delta_ms,
        (unsigned long)delta_frac);

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
        statistics_count_crc_error();
        
        printf("CRC ERROR\r\n");
        return;
    }

    if(packet->length == 3 &&
       packet->data[0] == 0xFF &&
       packet->data[1] == 0x00 &&
       packet->data[2] == 0xFF)
    {
        statistics_count_idle();
        
        printf("Idle");
    }
    else if(packet->data[0] == 0x00)
    {
        statistics_count_reset();
        
        printf("Reset");
    }
    else if(is_service_packet(packet))
    {
        statistics_count_service();
        
        analyze_service(packet);
    }
        else if(is_ops_packet(packet))
    {
        statistics_count_ops();
        
        analyze_ops(packet);
    }
    else if((packet->data[0] & 0xC0) == 0x80)
    {
        statistics_count_accessory();
        
        analyze_accessory(packet);
    }
    else
    {
        statistics_count_loco();
        
        analyze_loco(packet);
    }

    printf("\r\n");
}