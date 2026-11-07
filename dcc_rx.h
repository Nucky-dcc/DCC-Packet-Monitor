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
 * @file dcc_rx.h
 * @brief DCC信号受信モジュール
 *
 * GPIO入力からDCC信号を監視し、
 * エッジ情報をリングバッファへ格納する。
 *
 * 本モジュールの責務
 *
 * - DCC入力監視
 * - エッジ検出
 * - パルス幅測定
 * - タイムスタンプ取得
 * - リングバッファ管理
 *
 * DCC信号の解釈は行わず、
 * 生のエッジ情報のみを出力する。
 */

#ifndef DCC_RX_H
#define DCC_RX_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief エッジバッファサイズ
 *
 * DCC信号は高速に変化するため、
 * 十分なサイズを確保している。
 */
#define DCC_RX_BUFFER_SIZE 2048

/**
 * @brief DCCエッジ情報
 *
 * GPIOレベル変化発生時の情報を保持する。
 */
typedef struct
{
    /**
     * @brief エッジ発生時刻
     *
     * 単位: us
     */
    uint32_t timestamp_us;

    /**
     * @brief 前回エッジからのパルス幅
     *
     * 単位: us
     */
    uint32_t width_us;

    /**
     * @brief エッジ後のGPIOレベル
     *
     * 0 = Low
     * 1 = High
     */
    uint8_t level;

} DCC_EDGE;

/**
 * @brief DCC受信初期化
 *
 * GPIO設定および内部状態を初期化する。
 */
void dcc_init(void);

/**
 * @brief DCC受信処理
 *
 * GPIO状態を監視し、
 * レベル変化を検出した場合は
 * リングバッファへ記録する。
 *
 * 高速周期で呼び出すことを前提とする。
 */
void dcc_task(void);

/**
 * @brief エッジ取得
 *
 * バッファに格納されたエッジ情報を
 * 1件取り出す。
 *
 * @param edge 取得先
 *
 * @retval true  エッジ取得成功
 * @retval false データなし
 */
bool dcc_get_edge(
    DCC_EDGE *edge);

#endif