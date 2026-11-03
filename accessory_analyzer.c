#include <stdio.h>
#include <stdint.h>

#include "accessory_analyzer.h"

void analyze_accessory(
    const DCC_PACKET *packet)
{
    uint16_t addr_9bit;
    uint16_t display_addr;
    uint8_t pair;

    if(packet->length < 3)
    {
        printf("Accessory Packet");
        return;
    }

    addr_9bit =
        ((uint16_t)
         ((~packet->data[1]) & 0x70)
         << 2)
        |
        (packet->data[0] & 0x3F);

    pair =
        (packet->data[1] >> 1) & 0x03;

    display_addr =
        (addr_9bit > 0)
        ? ((addr_9bit - 1) * 4)
            + pair + 1
        : pair + 1;

    printf("Acc:%u ",
           display_addr);

    printf("%s",
           (packet->data[1] & 0x01)
           ? "Diverging"
           : "Straight");
}