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
 * @file dcc_decode.c
 * @brief DCC信号デコード処理
 *
 * dcc_rx が取得したエッジ情報から
 * DCC Bit(0/1) を復元する。
 *
 * 処理の流れ
 *
 * DCC Edge
 * ↓
 * パルス幅測定
 * ↓
 * ノイズ除去
 * ↓
 * Bit判定
 * ↓
 * Bitリングバッファ格納
 *
 * DCC Packet の復元は
 * dcc_packet モジュールが担当する。
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "dcc_rx.h"
#include "dcc_decode.h"
#include "statistics.h"
#include "monitor_config.h"

/**
 * @brief Bitリングバッファサイズ
 *
 * エッジ受信とパケット解析を分離するため、
 * 十分な余裕を確保している。
 */
#define DCC_BIT_BUFFER_SIZE 1024

/**
 * @brief Bitリングバッファ
 */
static DCC_BIT bit_buffer[DCC_BIT_BUFFER_SIZE];

/**
 * @brief 書き込み位置
 */
static volatile uint16_t bit_head;

/**
 * @brief 読み出し位置
 */
static volatile uint16_t bit_tail;

/**
 * @brief DCCデコーダ初期化
 *
 * Bitバッファをクリアする。
 */
void dcc_decode_init(void)
{
    memset(
        bit_buffer,
        0,
        sizeof(bit_buffer));

    bit_head = 0;
    bit_tail = 0;
}

/**
 * @brief DCCデコード処理
 *
 * dcc_rx から取得したエッジ情報を
 * DCC Bitへ変換する。
 *
 * Bit判定は
 * monitor_config.threshold_us
 * により実施する。
 */
void dcc_decode_task(void)
{
    DCC_EDGE edge;

    while(dcc_get_edge(&edge))
    {
        uint8_t bit_value;
        uint16_t next_head;

        /*
         * CH32V003版と同じ設計。
         *
         * LOWエッジのみ使用する。
         *
         * DCCではハーフビット幅が
         * 同じになるため、
         * 片側エッジのみで十分判定可能。
         */
        if(edge.level != 0)
        {
            continue;
        }

        /*
         * パルス幅ヒストグラム収集。
         *
         * しきい値評価や
         * 波形品質解析に利用する。
         */
        statistics_add_histogram(
            edge.width_us);

        /*
         * ノイズ除去
         *
         * DCCの規格上、
         * 20us未満のパルスは
         * 実信号としては扱わない。
         */
        if(edge.width_us < 20)
        {
            continue;
        }

        /*
         * DCC Bit判定
         *
         * Bit1:
         *   短いパルス
         *
         * Bit0:
         *   長いパルス
         *
         * 閾値はユーザー設定可能。
         */
        if(edge.width_us <=
           monitor_config.threshold_us)
        {
            bit_value = 1;

            statistics_count_bit1();

            statistics_add_bit1_width(
                edge.width_us);
        }
        else
        {
            bit_value = 0;

            statistics_count_bit0();

            statistics_add_bit0_width(
                edge.width_us);
        }

        /*
         * 次の書き込み位置計算
         */
        next_head =
            (bit_head + 1)
            % DCC_BIT_BUFFER_SIZE;

        /*
         * バッファ満杯。
         *
         * 最古データは保持し、
         * 新規データを破棄する。
         */
        if(next_head == bit_tail)
        {
            continue;
        }

        /*
         * Bit情報保存
         */
        bit_buffer[bit_head].timestamp_us =
            edge.timestamp_us;

        bit_buffer[bit_head].bit =
            bit_value;

        bit_head = next_head;
    }
}

/**
 * @brief DCC Bit取得
 *
 * FIFO形式でBitデータを取り出す。
 *
 * @param bit 取得先
 *
 * @retval true  データあり
 * @retval false データなし
 */
bool dcc_get_bit(
    DCC_BIT *bit)
{
    /*
     * バッファ空
     */
    if(bit_head == bit_tail)
    {
        return false;
    }

    /*
     * データ取得
     */
    *bit =
        bit_buffer[bit_tail];

    /*
     * 読み出し位置更新
     */
    bit_tail =
        (bit_tail + 1)
        % DCC_BIT_BUFFER_SIZE;

    return true;
}