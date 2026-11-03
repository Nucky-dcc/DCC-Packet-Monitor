#include <string.h>

#include "dcc_decode.h"
#include "dcc_packet.h"

#define DCC_PREAMBLE_MIN_BITS   14

typedef enum
{
    PACKET_STATE_PREAMBLE,
    PACKET_STATE_BYTE,
    PACKET_STATE_SEPARATOR

} PACKET_STATE;

static PACKET_STATE state;

static uint16_t preamble_count;

static uint8_t current_byte;
static uint8_t bit_count;

static DCC_PACKET current_packet;

static DCC_PACKET packet_buffer[DCC_PACKET_BUFFER_SIZE];

static volatile uint16_t packet_head;
static volatile uint16_t packet_tail;

static void packet_complete(void)
{
    uint16_t next_head;

    if(current_packet.length == 0)
    {
        return;
    }

    next_head =
        (packet_head + 1)
        % DCC_PACKET_BUFFER_SIZE;

    if(next_head == packet_tail)
    {
        return;
    }

    packet_buffer[packet_head] =
        current_packet;

    packet_head = next_head;
}

void dcc_packet_init(void)
{
    memset(packet_buffer,
           0,
           sizeof(packet_buffer));

    memset(&current_packet,
           0,
           sizeof(current_packet));

    packet_head = 0;
    packet_tail = 0;

    state = PACKET_STATE_PREAMBLE;

    preamble_count = 0;
    bit_count = 0;
    current_byte = 0;
}

void dcc_packet_task(void)
{
    DCC_BIT bit;

    while(dcc_get_bit(&bit))
    {
        switch(state)
        {
            case PACKET_STATE_PREAMBLE:

                if(bit.bit)
                {
                    if(preamble_count < 255)
                    {
                        preamble_count++;
                    }
                }
                else
                {
                    if(preamble_count >= DCC_PREAMBLE_MIN_BITS)
                    {
                        memset(&current_packet,
                               0,
                               sizeof(current_packet));

                        current_packet.timestamp_us =
                            bit.timestamp_us;

                        bit_count = 0;
                        current_byte = 0;

                        state =
                            PACKET_STATE_BYTE;
                    }

                    preamble_count = 0;
                }

                break;

            case PACKET_STATE_BYTE:

                current_byte =
                    (current_byte << 1)
                    | bit.bit;

                bit_count++;

                if(bit_count >= 8)
                {
                    if(current_packet.length <
                       DCC_PACKET_MAX_BYTES)
                    {
                        current_packet.data[
                            current_packet.length
                        ] = current_byte;

                        current_packet.length++;
                    }

                    bit_count = 0;
                    current_byte = 0;

                    state =
                        PACKET_STATE_SEPARATOR;
                }

                break;

            case PACKET_STATE_SEPARATOR:

                /*
                 * 0 = 次のデータ
                 * 1 = パケット終了
                 */

                if(bit.bit == 0)
                {
                    state =
                        PACKET_STATE_BYTE;
                }
                else
                {
                    packet_complete();

                    state =
                        PACKET_STATE_PREAMBLE;

                    preamble_count = 1;
                }

                break;

            default:

                state =
                    PACKET_STATE_PREAMBLE;

                preamble_count = 0;

                break;
        }
    }
}

bool dcc_get_packet(DCC_PACKET *packet)
{
    if(packet_head == packet_tail)
    {
        return false;
    }

    *packet =
        packet_buffer[packet_tail];

    packet_tail =
        (packet_tail + 1)
        % DCC_PACKET_BUFFER_SIZE;

    return true;
}