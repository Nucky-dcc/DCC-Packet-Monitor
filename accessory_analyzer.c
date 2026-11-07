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
 * @file accessory_analyzer.c
 * @brief アクセサリデコーダパケット解析
 *
 * NMRA Basic Accessory Decoder Packet を解析し、
 * ポイントや信号機の制御内容を表示する。
 *
 * 本モジュールは
 *
 * - アクセサリアドレス復元
 * - 出力番号復元
 * - 分岐方向表示
 *
 * を担当する。
 */

#include <stdio.h>
#include <stdint.h>

#include "accessory_analyzer.h"

/**
 * @brief Accessory Packet解析
 *
 * 表示例
 *
 * Acc:1 Straight
 * Acc:2 Diverging
 * Acc:1017 Straight
 *
 * NMRAアクセサリアドレスは
 * デコーダアドレスと出力番号の組で
 * 符号化されているため、
 * ユーザーが一般的に使用する
 * 連番アドレスへ変換して表示する。
 *
 * @param packet 解析対象パケット
 */
void analyze_accessory(
    const DCC_PACKET *packet)
{
    uint16_t addr_9bit;
    uint16_t display_addr;
    uint8_t pair;

    /*
     * 最低限必要な長さチェック
     */
    if(packet->length < 3)
    {
        printf("Accessory Packet");
        return;
    }

    /*
     * NMRA Basic Accessory Address復元
     *
     * A0～A5 は1バイト目、
     * A6～A8 は2バイト目へ反転状態で
     * 格納されている。
     *
     * そのため上位アドレス部は
     * ビット反転して復元する。
     */
    addr_9bit =
        ((uint16_t)
         ((~packet->data[1]) & 0x70)
         << 2)
        |
        (packet->data[0] & 0x3F);

    /*
     * 出力番号抽出
     *
     * 1つのアクセサリアドレスは
     * 4出力を持つ。
     *
     * pair:
     * 0～3
     */
    pair =
        (packet->data[1] >> 1) & 0x03;

    /*
     * NMRA内部表現を
     * ユーザー向けアドレスへ変換する。
     *
     * 例
     *
     * Address 1～4
     * Address 5～8
     * Address 9～12
     *
     * のような連番へ変換する。
     *
     * この計算は packet_analyzer.c の
     * アドレスフィルタ処理と
     * 一致させる必要がある。
     */
    display_addr =
        (addr_9bit > 0)
        ? ((addr_9bit - 1) * 4)
            + pair + 1
        : pair + 1;

    /*
     * アドレス表示
     */
    printf("Acc:%u ",
           display_addr);

    /*
     * 動作方向表示
     *
     * bit0:
     * 0 = Straight
     * 1 = Diverging
     */
    printf("%s",
           (packet->data[1] & 0x01)
           ? "Diverging"
           : "Straight");
}