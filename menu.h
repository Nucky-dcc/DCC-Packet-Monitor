#include <stdbool.h>

#ifndef MENU_H
#define MENU_H

typedef enum
{
    MENU_NONE,

    MENU_MAIN,

    MENU_MONITOR,
    MENU_PACKET_STATS,
    MENU_SIGNAL_STATS,
    MENU_SIGNAL_HISTOGRAM,

    MENU_SETTINGS,

    MENU_FILTER,
    MENU_ADDRESS

} MENU_STATE;

/*
 * メインメニュー表示
 */
void menu_show_main(void);

/*
 * キー入力処理
 */
void menu_process_key(int ch);

bool menu_is_monitor_mode(void);

#endif
