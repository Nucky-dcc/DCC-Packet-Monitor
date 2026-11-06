#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "menu.h"
#include "monitor_config.h"
#include "packet_analyzer.h"
#include "dcc_packet.h"
#include "statistics.h"

static MENU_STATE current_menu =
    MENU_NONE;

static char address_input[5];

static uint8_t address_input_index = 0;

static const char *filter_name[] =
{
    "全パケット",
    "機関車",
    "アクセサリ",
    "OPS"
};

static const char *onoff(bool value);

static const char *address_filter_name(void);

static void menu_print_config(void)
{
    printf("--------------------------------\r\n");

    printf("フィルタ    : %s\r\n",
           filter_name[monitor_config.filter]);

    if(monitor_config.address_filter == 0)
    {
        printf("アドレス : 無効\r\n");
    }
    else
    {
        printf("アドレス : %u\r\n",
            monitor_config.address_filter);
    }

    printf("RESET表示   : %s\r\n",
           monitor_config.show_reset ?
           "ON" : "OFF");

    printf("IDLE表示    : %s\r\n",
           monitor_config.show_idle ?
           "ON" : "OFF");

    printf("--------------------------------\r\n");
}

void menu_show_main(void)
{
    current_menu = MENU_MAIN;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" DCCパケットモニタ\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("1. パケット監視\r\n");
    printf("2. パケット統計\r\n");
    printf("3. 信号統計\r\n");
    printf("4. 設定\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");
    printf("選択 > ");
}

static void menu_show_monitor(void)
{
    current_menu = MENU_MONITOR;

    dcc_packet_init();
    packet_analyzer_init();

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" パケット監視\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("現在設定で監視を開始します\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");
    printf("ESC : 戻る\r\n");
    printf("\r\n");
}

static void menu_show_packet_stats(void)
{
    uint64_t elapsed_us;

    uint32_t elapsed_sec;
    uint32_t elapsed_ms;

    uint32_t packets_per_sec;

    current_menu = MENU_PACKET_STATS;

    elapsed_us =
        packet_statistics.elapsed_us;

    elapsed_sec =
        (uint32_t)
        (elapsed_us / 1000000ULL);

    elapsed_ms =
        (uint32_t)
        ((elapsed_us % 1000000ULL)
        / 1000ULL);

    if(elapsed_us == 0)
    {
        packets_per_sec = 0;
    }
    else
    {
        packets_per_sec =
            (uint32_t)
            (((uint64_t)
            packet_statistics.total
            * 1000000ULL)
            / elapsed_us);
    }

    printf("\r\n\r\n");
    printf("================================\r\n");
    printf(" パケット統計\r\n");
    printf("================================\r\n");
    printf("\r\n");

    printf(
        "測定時間    : %lu.%03lus\r\n",
        (unsigned long)elapsed_sec,
        (unsigned long)elapsed_ms);

    printf(
        "平均速度    : %lu pkt/s\r\n",
        (unsigned long)packets_per_sec);

    printf("\r\n");

    printf("Total       : %lu\r\n",
        (unsigned long)
        packet_statistics.total);

    printf("\r\n");

    printf("Reset       : %lu\r\n",
        (unsigned long)
        packet_statistics.reset);

    printf("Idle        : %lu\r\n",
        (unsigned long)
        packet_statistics.idle);

    printf("\r\n");

    printf("Loco        : %lu\r\n",
        (unsigned long)
        packet_statistics.loco);

    printf("Accessory   : %lu\r\n",
        (unsigned long)
        packet_statistics.accessory);

    printf("\r\n");

    printf("OPS         : %lu\r\n",
        (unsigned long)
        packet_statistics.ops);

    printf("Service     : %lu\r\n",
        (unsigned long)
        packet_statistics.service);

    printf("\r\n");

    printf("CRC Error   : %lu\r\n",
        (unsigned long)
        packet_statistics.crc_error);

    printf("\r\n");

    printf("R   : 統計クリア\r\n");
    printf("ESC : 戻る\r\n");
}


static void menu_show_signal_stats(void)
{
    current_menu = MENU_SIGNAL_STATS;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" 信号統計\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("信号統計機能は未実装\r\n");

    printf("\r\n");
    printf("ESC : 戻る\r\n");
}

static void menu_show_settings(void)
{
    current_menu = MENU_SETTINGS;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" 設定\r\n");
    printf("================================\r\n");
    printf("\r\n");

    printf("1. 表示フィルタ      [%s]\r\n",
           filter_name[monitor_config.filter]);

    printf("2. アドレスフィルタ  [%s]\r\n",
           address_filter_name());

    printf("3. RESET表示         [%s]\r\n",
           onoff(monitor_config.show_reset));

    printf("4. IDLE表示          [%s]\r\n",
           onoff(monitor_config.show_idle));

    printf("5. 判定閾値          [%uus]\r\n",
           monitor_config.threshold_us);

    printf("6. プリアンブル      [%u]\r\n",
           monitor_config.preamble_count);

    printf("\r\n");

    printf("ESC : 戻る\r\n");

    printf("\r\n");
    printf("選択 > ");
}

static void menu_show_filter(void)
{
    current_menu = MENU_FILTER;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" 表示フィルタ\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("現在 : %s\r\n",
           filter_name[monitor_config.filter]);

    printf("\r\n");

    printf("1. 全パケット\r\n");
    printf("2. 機関車\r\n");
    printf("3. アクセサリ\r\n");
    printf("4. OPS\r\n");

    printf("\r\n");

    printf("ESC : 戻る\r\n");
    printf("\r\n");

    printf("選択 > ");
}

static void menu_show_address(void)
{
    current_menu = MENU_ADDRESS;

    address_input_index = 0;
    address_input[0] = '\0';

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" アドレスフィルタ\r\n");
    printf("================================\r\n");

    printf("\r\n");

    if(monitor_config.address_filter == 0)
    {
        printf("現在 : 無効\r\n");
    }
    else
    {
        printf("現在 : %u\r\n",
               monitor_config.address_filter);
    }

    printf("\r\n");

    printf("0      : フィルタ無効\r\n");
    printf("1-9999 : アドレス指定\r\n");

    printf("\r\n");

    printf("ENTER : 変更なし\r\n");
    printf("ESC   : 戻る\r\n");

    printf("\r\n");

    printf("入力 > ");
}

void menu_process_key(int ch)
{
    if(ch == 27)
    {
        switch(current_menu)
        {
            case MENU_MAIN:
                break;

            case MENU_SETTINGS:
                menu_show_main();
                break;

            case MENU_FILTER:
                menu_show_settings();
                break;

            case MENU_ADDRESS:

                address_input_index = 0;
                address_input[0] = '\0';

                menu_show_settings();
                break;

            case MENU_MONITOR:
                menu_show_main();
                break;

            case MENU_PACKET_STATS:
                menu_show_main();
                break;

            case MENU_SIGNAL_STATS:
                menu_show_main();
                break;

            default:
                menu_show_main();
                break;
        }

        return;
    }
    
    switch(current_menu)
    {
        case MENU_NONE:

            if((ch == '\r') ||
            (ch == '\n'))
            {
                menu_show_main();
            }

            break;

        case MENU_MAIN:

            switch(ch)
            {
                case '1':
                    menu_show_monitor();
                    break;

                case '2':
                    menu_show_packet_stats();
                    break;

                case '3':
                    menu_show_signal_stats();
                    break;

                case '4':
                    menu_show_settings();
                    break;

                default:
                    break;
            }

            break;

        case MENU_MONITOR:
            break;

        case MENU_PACKET_STATS:

            if((ch == 'r') ||
            (ch == 'R'))
            {
                statistics_reset();

                menu_show_packet_stats();
            }

            break;

        case MENU_SIGNAL_STATS:
            break;

        case MENU_SETTINGS:

            switch(ch)
            {
                case '1':
                    menu_show_filter();
                    break;

                case '2':
                    menu_show_address();
                    break;

                case '3':

                    monitor_config.show_reset =
                        !monitor_config.show_reset;

                    menu_show_settings();

                    break;

                case '4':

                    monitor_config.show_idle =
                        !monitor_config.show_idle;

                    menu_show_settings();

                    break;

                default:
                    break;
            }

            break;

        case MENU_FILTER:

            switch(ch)
            {
                case '1':
                    monitor_config.filter =
                        FILTER_ALL;
                    break;

                case '2':
                    monitor_config.filter =
                        FILTER_LOCO;
                    break;

                case '3':
                    monitor_config.filter =
                        FILTER_ACCESSORY;
                    break;

                case '4':
                    monitor_config.filter =
                        FILTER_OPS;
                    break;

                default:
                    return;
            }

            menu_show_settings();

            break;

        case MENU_ADDRESS:

            if(ch == '\r' || ch == '\n')
            {
                if(address_input_index == 0)
                {
                    menu_show_settings();
                    return;
                }

                uint16_t value;

                value = (uint16_t)atoi(address_input);

                if(value <= 9999)
                {
                    monitor_config.address_filter = value;
                }

                address_input_index = 0;
                address_input[0] = '\0';

                menu_show_settings();

                return;
            }

            if((ch == 8) || (ch == 127))
            {
                if(address_input_index > 0)
                {
                    address_input_index--;

                    address_input[address_input_index] =
                        '\0';

                    printf("\b \b");
                }

                break;
            }

            if(ch >= '0' && ch <= '9')
            {
                if(address_input_index < 4)
                {
                    address_input[address_input_index++] =
                        (char)ch;

                    address_input[address_input_index] =
                        '\0';

                    putchar(ch);
                }
            }

            break;
                
        default:
            break;
        }
      
}

static const char *onoff(bool value)
{
    return value ? "ON" : "OFF";
}

static const char *address_filter_name(void)
{
    static char text[16];

    if(monitor_config.address_filter == 0)
    {
        return "無効";
    }

    snprintf(text,
             sizeof(text),
             "%u",
             monitor_config.address_filter);

    return text;
}

bool menu_is_monitor_mode(void)
{
    return (current_menu == MENU_MONITOR);
}