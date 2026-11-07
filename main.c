/*
 * DCC Packet Monitor
 *
 * Copyright (C) 2026 Nucky
 *
 * This program is free software:
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;
 * without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/**
 * @file main.c
 * @brief DCC Packet Monitor メインモジュール
 *
 * システム初期化およびメイン制御ループを実装する。
 *
 * 本モジュールは解析処理を持たず、
 * 各モジュールを順番に実行する役割のみを担当する。
 *
 * データフロー
 *
 * DCC Signal
 *   ↓
 * dcc_rx
 *   ↓
 * dcc_decode
 *   ↓
 * dcc_packet
 *   ↓
 * packet_analyzer
 *
 * 設計方針
 *
 * 処理責務を明確化するため、
 * 各段階を独立モジュールとして分離している。
 */

#include <stdio.h>

#include "pico/stdlib.h"

#include "dcc_rx.h"
#include "dcc_decode.h"
#include "dcc_packet.h"

#include "packet_analyzer.h"

#include "monitor_config.h"
#include "menu.h"

/**
 * @brief エントリポイント
 *
 * システム初期化後、
 * 無限ループ内で各モジュールを実行する。
 *
 * メインループは以下の順序で動作する。
 *
 * 1. キー入力取得
 * 2. DCCエッジ取得
 * 3. Bit変換
 * 4. Packet復元
 * 5. Packet解析
 *
 * この順序により、
 * 信号取得層と解析層を完全に分離している。
 *
 * @return 終了しない
 */
int main(void)
{
    /**
     * USB CDC初期化
     *
     * コンソール入出力に使用する。
     */
    stdio_init_all();

    /**
     * Pico標準LED初期化
     *
     * 電源投入確認用。
     */
    gpio_init(PICO_DEFAULT_LED_PIN);

    gpio_set_dir(
        PICO_DEFAULT_LED_PIN,
        GPIO_OUT);

    gpio_put(
        PICO_DEFAULT_LED_PIN,
        1);

    printf("\r\n");
    printf("DCC Packet Monitor\r\n");

    /**
     * 受信系初期化
     */
    dcc_init();
    dcc_decode_init();
    dcc_packet_init();

    /**
     * 解析系初期化
     */
    packet_analyzer_init();

    /**
     * 設定初期化
     */
    monitor_config_init();

    printf("\r\n");
    printf("DCCパケットモニタ\r\n");
    printf("ENTERでメニュー表示\r\n");

    while (true)
    {
        int ch;
        DCC_PACKET packet;

        /**
         * コンソール入力取得
         *
         * タイムアウト0により
         * ノンブロッキングで動作する。
         */
        ch = getchar_timeout_us(0);

        if (ch >= 0)
        {
            menu_process_key(ch);
        }

        /**
         * DCC受信パイプライン
         *
         * dcc_rx
         *   ↓
         * dcc_decode
         *   ↓
         * dcc_packet
         */
        dcc_task();
        dcc_decode_task();
        dcc_packet_task();

        /**
         * 復元済みパケット処理
         */
        while (dcc_get_packet(&packet))
        {
            /**
             * モニタ画面表示中のみ
             * パケット解析・表示を実施する。
             *
             * パケット取得自体は継続して行い、
             * FIFOオーバーフローを防止する。
             */
            if (menu_is_monitor_mode())
            {
                packet_analyzer(&packet);
            }
        }
    }

    /* 到達しない */
    return 0;
}