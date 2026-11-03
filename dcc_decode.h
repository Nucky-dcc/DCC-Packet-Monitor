#ifndef DCC_DECODE_H
#define DCC_DECODE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t timestamp_us;
    uint8_t bit;
} DCC_BIT;

void dcc_decode_init(void);
void dcc_decode_task(void);

bool dcc_get_bit(DCC_BIT *bit);

#endif