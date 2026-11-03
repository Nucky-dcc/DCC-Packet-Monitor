#include <stdio.h>

#include "pico/stdlib.h"

#include "dcc_rx.h"
#include "dcc_decode.h"
#include "dcc_packet.h"
#include "packet_analyzer.h"

int main(void)
{
    stdio_init_all();

    sleep_ms(3000);

    printf("\r\n");
    printf("DCC Packet Monitor\r\n");

    dcc_init();

    dcc_decode_init();

    dcc_packet_init();

    packet_analyzer_init();

    while(true)
    {
        dcc_task();

        dcc_decode_task();

        dcc_packet_task();

        DCC_PACKET packet;

        while(dcc_get_packet(&packet))
        {
            packet_analyzer(&packet);
        }
    }
}