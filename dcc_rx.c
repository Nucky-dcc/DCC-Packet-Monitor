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
 * @file dcc_rx.c
 * @brief DCC信号受信モジュール
 *
 * GPIO入力からDCC信号を取得し、
 * エッジ情報へ変換する。
 *
 * 本モジュールは
 *
 * GPIO
 * ↓
 * エッジ検出
 * ↓
 * パルス幅測定
 * ↓
 * リングバッファ
 *
 * までを担当する。
 *
 * DCCビット判定やパケット復元は
 * dcc_decode および dcc_packet が担当する。
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "dcc_rx.h"

/**
 * @brief DCC入力ピン
 *
 * フォトカプラ出力などを接続する。
 */
#define DCC_PIN 2

/**
 * @brief エッジ情報リングバッファ
 */
static DCC_EDGE rx_buffer[DCC_RX_BUFFER_SIZE];

/**
 * @brief 書き込み位置
 */
static volatile uint16_t rx_head = 0;

/**
 * @brief 読み出し位置
 */
static volatile uint16_t rx_tail = 0;

/**
 * @brief 前回GPIO状態
 *
 * エッジ検出に使用する。
 */
static int last_state;

/**
 * @brief 前回エッジ発生時刻
 *
 * パルス幅計算基準。
 */
static uint32_t last_time;

/**
 * @brief DCC受信初期化
 *
 * GPIOを入力モードに設定し、
 * バッファおよび内部状態を初期化する。
 */
void dcc_init(void)
{
    gpio_init(DCC_PIN);

    gpio_set_dir(
        DCC_PIN,
        GPIO_IN);

    memset(
        rx_buffer,
        0,
        sizeof(rx_buffer));

    rx_head = 0;
    rx_tail = 0;

    /*
     * 現在状態を初期値として保存。
     */
    last_state =
        gpio_get(DCC_PIN);

    last_time =
        time_us_32();
}

/**
 * @brief DCC受信処理
 *
 * GPIO状態変化を監視する。
 *
 * 状態変化を検出すると
 *
 * - エッジ発生時刻
 * - パルス幅
 * - レベル
 *
 * をリングバッファへ格納する。
 */
void dcc_task(void)
{
    int state =
        gpio_get(DCC_PIN);

    /*
     * レベル変化検出
     */
    if(state != last_state)
    {
        uint32_t now =
            time_us_32();

        /*
         * 前回エッジからの経過時間。
         */
        uint32_t width =
            now - last_time;

        /*
         * リングバッファ次位置。
         */
        uint16_t next_head =
            (rx_head + 1) %
            DCC_RX_BUFFER_SIZE;

        /*
         * バッファ満杯時は
         * 新規データを破棄する。
         */
        if(next_head != rx_tail)
        {
            rx_buffer[rx_head].timestamp_us =
                now;

            rx_buffer[rx_head].width_us =
                width;

            rx_buffer[rx_head].level =
                (uint8_t)state;

            rx_head = next_head;
        }

        /*
         * 次回計測基準更新。
         */
        last_time = now;
        last_state = state;
    }
}

/**
 * @brief エッジ情報取得
 *
 * リングバッファから
 * 1件取り出す。
 *
 * FIFOとして動作する。
 *
 * @param edge 取得先
 *
 * @retval true  データあり
 * @retval false データなし
 */
bool dcc_get_edge(
    DCC_EDGE *edge)
{
    /*
     * バッファ空。
     */
    if(rx_head == rx_tail)
    {
        return false;
    }

    /*
     * データ取得。
     */
    *edge =
        rx_buffer[rx_tail];

    /*
     * 読み出し位置更新。
     */
    rx_tail =
        (rx_tail + 1) %
        DCC_RX_BUFFER_SIZE;

    return true;
}