#include <stdio.h>

#include "pico/stdlib.h"

#include "dcc_rx.h"
#include "dcc_decode.h"
#include "dcc_packet.h"
#include "packet_analyzer.h"
#include "monitor_config.h"
#include "menu.h"

int main(void)
{
    stdio_init_all();

    printf("\r\n");
    printf("DCC Packet Monitor\r\n");

    dcc_init();
    dcc_decode_init();
    dcc_packet_init();
    packet_analyzer_init();

    monitor_config_init();

    printf("\r\n");
    printf("DCCパケットモニタ\r\n");
    printf("ENTERでメニュー表示\r\n");

    while(true)
    {
        int ch;
        DCC_PACKET packet;

        ch = getchar_timeout_us(0);

        if(ch >= 0)
        {
            menu_process_key(ch);
        }

        dcc_task();

        dcc_decode_task();

        dcc_packet_task();

        while(dcc_get_packet(&packet))
        {
            if(menu_is_monitor_mode())
            {
                packet_analyzer(&packet);
            }
        }
    }

    return 0;
}