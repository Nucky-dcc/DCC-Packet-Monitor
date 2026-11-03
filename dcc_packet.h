#ifndef DCC_PACKET_H
#define DCC_PACKET_H

#include <stdint.h>
#include <stdbool.h>

#define DCC_PACKET_MAX_BYTES    8
#define DCC_PACKET_BUFFER_SIZE  32

typedef struct
{
    uint32_t timestamp_us;

    uint8_t data[DCC_PACKET_MAX_BYTES];

    uint8_t length;

} DCC_PACKET;

void dcc_packet_init(void);

void dcc_packet_task(void);

bool dcc_get_packet(DCC_PACKET *packet);

#endif