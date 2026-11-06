#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "dcc_rx.h"
#include "dcc_decode.h"
#include "statistics.h"

#define DCC_BIT_BUFFER_SIZE     1024

/* CH32V003版と合わせる */
#define DCC_ONE_THRESHOLD_US    90

static DCC_BIT bit_buffer[DCC_BIT_BUFFER_SIZE];

static volatile uint16_t bit_head;
static volatile uint16_t bit_tail;

void dcc_decode_init(void)
{
    memset(bit_buffer, 0, sizeof(bit_buffer));

    bit_head = 0;
    bit_tail = 0;
}

void dcc_decode_task(void)
{
    DCC_EDGE edge;

    while(dcc_get_edge(&edge))
    {
        uint8_t bit_value;
        uint16_t next_head;

        /*
         * CH32V003版と同じ考え方
         *
         * LOWになったエッジのみ採用
         */

        if(edge.level != 0)
        {
            continue;
        }

        /*
         * ノイズ除去
         */

        if(edge.width_us < 20)
        {
            continue;
        }

        /*
         * DCC Bit判定
         */

        if(edge.width_us <= DCC_ONE_THRESHOLD_US)
        {
            bit_value = 1;

            statistics_count_bit1();

            statistics_add_bit1_width(
                edge.width_us);
        }
        else
        {
            bit_value = 0;

            statistics_count_bit0();

            statistics_add_bit0_width(
                edge.width_us);
        }

        next_head =
            (bit_head + 1)
            % DCC_BIT_BUFFER_SIZE;

        if(next_head == bit_tail)
        {
            continue;
        }

        bit_buffer[bit_head].timestamp_us =
            edge.timestamp_us;

        bit_buffer[bit_head].bit =
            bit_value;

        bit_head = next_head;
    }
}

bool dcc_get_bit(DCC_BIT *bit)
{
    if(bit_head == bit_tail)
    {
        return false;
    }

    *bit = bit_buffer[bit_tail];

    bit_tail =
        (bit_tail + 1)
        % DCC_BIT_BUFFER_SIZE;

    return true;
}