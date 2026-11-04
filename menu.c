#include <stdio.h>

#include "menu.h"
#include "monitor_config.h"

static MENU_STATE current_menu =
    MENU_NONE;

static const char *filter_name[] =
{
    "全パケット",
    "機関車",
    "アクセサリ",
    "OPS"
};

static void menu_print_config(void)
{
    printf("--------------------------------\r\n");

    printf("フィルタ    : %s\r\n",
           filter_name[monitor_config.filter]);

    if(monitor_config.address_filter_enable)
    {
        printf("アドレス    : %u\r\n",
               monitor_config.address_filter);
    }
    else
    {
        printf("アドレス    : 無効\r\n");
    }

    printf("RESET表示   : %s\r\n",
           monitor_config.show_reset ?
           "ON" : "OFF");

    printf("IDLE表示    : %s\r\n",
           monitor_config.show_idle ?
           "ON" : "OFF");

    printf("判定閾値    : %uus\r\n",
           monitor_config.threshold_us);

    printf("プリアンブル: %u\r\n",
           monitor_config.preamble_count);

    printf("--------------------------------\r\n");
}

void menu_show_main(void)
{
    current_menu = MENU_MAIN;

    printf("\r\n");

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

    printf("\r\n");

    printf("================================\r\n");
    printf(" パケット監視\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("現在設定で監視を開始します\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");
    printf("ESC : 戻る\r\n");
}

static void menu_show_packet_stats(void)
{
    current_menu = MENU_PACKET_STATS;

    printf("\r\n");

    printf("================================\r\n");
    printf(" パケット統計\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("統計機能は未実装\r\n");

    printf("\r\n");
    printf("ESC : 戻る\r\n");
}

static void menu_show_signal_stats(void)
{
    current_menu = MENU_SIGNAL_STATS;

    printf("\r\n");

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

    printf("\r\n");

    printf("================================\r\n");
    printf(" 設定\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf("1. 表示フィルタ\r\n");
    printf("2. アドレスフィルタ\r\n");
    printf("3. RESET表示\r\n");
    printf("4. IDLE表示\r\n");
    printf("5. 判定閾値\r\n");
    printf("6. プリアンブル\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");
    printf("ESC : 戻る\r\n");
    printf("\r\n");
    printf("選択 > ");
}

static void menu_show_filter(void)
{
    current_menu = MENU_FILTER;

    printf("\r\n");

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

void menu_process_key(int ch)
{
    /*
     * ESC
     */
    if(ch == 27)
    {
        menu_show_main();
        return;
    }

    switch(current_menu)
    {
        case MENU_NONE:
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
            break;

        case MENU_SIGNAL_STATS:
            break;

        case MENU_SETTINGS:

            switch(ch)
            {
                case '1':
                    menu_show_filter();
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
            
            }
}