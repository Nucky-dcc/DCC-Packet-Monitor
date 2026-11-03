#include <stdio.h>
#include <stdint.h>

#include "loco_analyzer.h"

static void print_function_bits(
    uint8_t data,
    uint8_t count)
{
    for(uint8_t i = 0;
        i < count;
        i++)
    {
        printf("%c",
               (data & (1 << i))
               ? '*'
               : '_');

        if(count == 8 && i == 3)
        {
            printf(" ");
        }
    }
}

void analyze_loco(
    const DCC_PACKET *packet)
{
    uint16_t address;
    uint8_t inst;
    uint8_t data;

    if((packet->data[0] & 0xC0) == 0xC0)
    {
        if(packet->length < 4)
        {
            printf("Long Address Packet");
            return;
        }

        address =
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             | packet->data[1];

        inst = packet->data[2];
        data = packet->data[3];
    }
    else
    {
        if(packet->length < 3)
        {
            printf("Short Address Packet");
            return;
        }

        address = packet->data[0];
        inst    = packet->data[1];
        data    = packet->data[2];
    }

    printf("Loco:%u ",
           address);

    if(inst == 0x3F)
    {
        printf("[128SPD] ");

        printf("%s ",
               (data & 0x80)
               ? "FWD"
               : "REV");

        printf("%u",
               data & 0x7F);
    }
    else if((inst & 0xE0) == 0x80)
    {
        printf("[F0-F4 ] ");

        printf("%c",
               (inst & 0x10) ? '*' : '_');

        printf("%c",
               (inst & 0x01) ? '*' : '_');

        printf("%c",
               (inst & 0x02) ? '*' : '_');

        printf("%c",
               (inst & 0x04) ? '*' : '_');

        printf("%c",
               (inst & 0x08) ? '*' : '_');
    }
    else if((inst & 0xF0) == 0xB0)
    {
        printf("[F5-F8 ] ");

        print_function_bits(
            inst,
            4);
    }
    else if((inst & 0xF0) == 0xA0)
    {
        printf("[F9-F12] ");

        print_function_bits(
            inst,
            4);
    }
    else if(inst == 0xDE ||
            inst == 0xDF ||
           (inst >= 0xD8 &&
            inst <= 0xDC))
    {
        uint8_t start_f;

        start_f =
            (inst == 0xDE)
            ? 13
            : (inst == 0xDF)
                ? 21
                : 29 +
                  (inst - 0xD8) * 8;

        printf("[F%u-F%u] ",
               start_f,
               start_f + 7);

        print_function_bits(
            data,
            8);
    }
    else
    {
        printf("CMD=%02X",
               inst);
    }
}