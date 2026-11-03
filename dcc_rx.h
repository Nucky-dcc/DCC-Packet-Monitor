#ifndef DCC_RX_H
#define DCC_RX_H

#include <stdint.h>
#include <stdbool.h>

#define DCC_RX_BUFFER_SIZE 2048

typedef struct
{
    uint32_t timestamp_us;
    uint16_t width_us;
    uint8_t level;
} DCC_EDGE;

void dcc_init(void);
void dcc_task(void);
bool dcc_get_edge(DCC_EDGE *edge);

#endif
