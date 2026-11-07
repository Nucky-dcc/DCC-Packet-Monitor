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
 * @file dcc_decode.h
 * @brief DCC信号デコード処理
 *
 * dcc_rx が取得したエッジ情報から
 * DCCのBit列を復元する。
 *
 * 本モジュールの責務
 *
 * - パルス幅判定
 * - DCC Bit復元
 * - ノイズ除去
 * - Bitリングバッファ管理
 *
 * DCCパケット復元は行わず、
 * Bit列生成までを担当する。
 */

#ifndef DCC_DECODE_H
#define DCC_DECODE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 復元済みDCC Bit
 *
 * dcc_packet が参照する中間データ。
 */
typedef struct
{
    /**
     * @brief Bit受信時刻
     *
     * 単位: us
     */
    uint32_t timestamp_us;

    /**
     * @brief DCC Bit値
     *
     * 0 = DCC Bit0
     * 1 = DCC Bit1
     */
    uint8_t bit;

} DCC_BIT;

/**
 * @brief DCCデコーダ初期化
 *
 * Bitリングバッファを初期化する。
 */
void dcc_decode_init(void);

/**
 * @brief DCCデコード処理
 *
 * dcc_rx のエッジ情報から
 * DCC Bit を生成する。
 */
void dcc_decode_task(void);

/**
 * @brief DCC Bit取得
 *
 * 復元済みBitを1件取得する。
 *
 * @param bit 取得先
 *
 * @retval true  取得成功
 * @retval false データなし
 */
bool dcc_get_bit(
    DCC_BIT *bit);

#endif