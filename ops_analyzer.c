#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ops_analyzer.h"

bool is_ops_packet(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    if(packet == NULL)
    {
        return false;
    }

    /*
     * Short Address
     */
    if((packet->data[0] & 0xC0) != 0xC0)
    {
        if(packet->length < 5)
        {
            return false;
        }

        inst = packet->data[1];
    }
    /*
     * Long Address
     */
    else
    {
        if(packet->length < 6)
        {
            return false;
        }

        inst = packet->data[2];
    }

    return
        ((inst & 0xF0) == 0xE0);
}

void analyze_ops(
    const DCC_PACKET *packet)
{
    uint16_t address;
    uint16_t cv;
    uint8_t inst;
    uint8_t cv_low;
    uint8_t data;

    /*
     * Short Address
     */
    if((packet->data[0] & 0xC0) != 0xC0)
    {
        address = packet->data[0];

        inst   = packet->data[1];
        cv_low = packet->data[2];
        data   = packet->data[3];
    }
    /*
     * Long Address
     */
    else
    {
        address =
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             | packet->data[1];

        inst   = packet->data[2];
        cv_low = packet->data[3];
        data   = packet->data[4];
    }

    cv =
        (((uint16_t)(inst & 0x03) << 8)
          | cv_low)
        + 1;

    printf(
        "OPS Addr%u CV%u Write=%u",
        address,
        cv,
        data);
}