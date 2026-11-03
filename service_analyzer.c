#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "service_analyzer.h"

static uint8_t current_page = 1;

bool is_service_packet(
    const DCC_PACKET *packet)
{
    if(packet == NULL)
    {
        return false;
    }

    if(packet->length < 3)
    {
        return false;
    }

    return ((packet->data[0] & 0xF0) == 0x70);
}

void analyze_service(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    inst = packet->data[0];

    printf("Prog ");

    /*--------------------------------------------------
     * Direct Mode
     *-------------------------------------------------*/
    if(packet->length >= 4)
    {
        uint16_t cv;

        cv =
            (((uint16_t)(inst & 0x03) << 8)
              | packet->data[1])
            + 1;

        if((inst & 0xF4) == 0x74)
        {
            printf("Direct Byte CV%u",
                   cv);

            printf(
                (inst & 0x08)
                ? " Write="
                : " Verify=");

            printf("%u",
                   packet->data[2]);

            return;
        }

        else if((inst & 0xFC) == 0x78)
        {
            uint8_t bit_pos;
            uint8_t bit_val;

            bit_pos =
                packet->data[2] & 0x07;

            bit_val =
                (packet->data[2] >> 3) & 0x01;

            printf("Direct Bit CV%u",
                   cv);

            printf(
                (packet->data[2] & 0x10)
                ? " Write "
                : " Verify ");

            printf("Bit%u=%u",
                   bit_pos,
                   bit_val);

            return;
        }

        printf("Direct Unknown");
        return;
    }

    /*--------------------------------------------------
     * Paged Mode
     *-------------------------------------------------*/
    if(packet->length == 3)
    {
        uint8_t reg;

        reg = inst & 0x07;

        if(reg == 5)
        {
            current_page =
                packet->data[1];

            printf(
                "PageReg=%u",
                current_page);

            return;
        }

        {
            uint16_t cv = 0;

            if(reg <= 3)
            {
                cv =
                    ((uint16_t)
                     (current_page - 1) * 4)
                    + reg + 1;
            }
            else if(reg == 4)
            {
                cv = 29;
            }
            else if(reg == 6)
            {
                cv = 7;
            }
            else if(reg == 7)
            {
                cv = 8;
            }

            if(cv > 0)
            {
                printf("Paged CV%u",
                       cv);

                printf(
                    (inst & 0x08)
                    ? " Write="
                    : " Verify=");

                printf("%u",
                       packet->data[1]);
            }
            else
            {
                printf("AddrMode CV1 ");

                printf(
                    (inst & 0x08)
                    ? "Write="
                    : "Verify=");

                printf("%u",
                       packet->data[1]);
            }
        }
    }
}