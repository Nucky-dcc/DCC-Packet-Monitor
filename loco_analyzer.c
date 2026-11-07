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
 * @file loco_analyzer.c
 * @brief 機関車パケット解析
 *
 * 機関車制御パケットを解析し、
 * 読みやすいテキスト形式で表示する。
 *
 * 対応内容
 *
 * - Long Address
 * - Short Address
 * - 128 Step Speed Command
 * - Function Group 1 (F0～F4)
 * - Function Group 2 (F5～F8)
 * - Function Group 3 (F9～F12)
 * - Function Expansion (F13～F68)
 *
 * 未対応コマンドは CMD=xx として表示する。
 */

#include <stdio.h>
#include <stdint.h>

#include "loco_analyzer.h"

/**
 * @brief Function状態表示
 *
 * ビット状態を
 *
 * * = ON
 * _ = OFF
 *
 * として表示する。
 *
 * 表示例
 *
 * ****
 * *__*
 *
 * @param data  Functionビット列
 * @param count 表示ビット数
 */
static void print_function_bits(
    uint8_t data,
    uint8_t count)
{
    for(uint8_t i = 0;
        i < count;
        i++)
    {
        printf("%c",
               (data & (1 << i))
               ? '*'
               : '_');

        /*
         * 8bit表示時は
         * 4bitごとに区切りを入れる。
         */
        if(count == 8 && i == 3)
        {
            printf(" ");
        }
    }
}

/**
 * @brief 機関車パケット解析
 *
 * 表示例
 *
 * Loco:3 [128SPD] FWD 20
 *
 * Loco:5 [F0-F4 ] *****
 *
 * Loco:123 [F13-F20] *__* *_**
 *
 * @param packet 解析対象パケット
 */
void analyze_loco(
    const DCC_PACKET *packet)
{
    uint16_t address;
    uint8_t inst;
    uint8_t data;

    /*
     * Long Address判定
     *
     * 上位2bit = 11
     */
    if((packet->data[0] & 0xC0) == 0xC0)
    {
        if(packet->length < 4)
        {
            printf("Long Address Packet");
            return;
        }

        /*
         * 14bitアドレス復元
         */
        address =
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             | packet->data[1];

        inst = packet->data[2];
        data = packet->data[3];
    }
    else
    {
        if(packet->length < 3)
        {
            printf("Short Address Packet");
            return;
        }

        /*
         * Short Address
         */
        address = packet->data[0];

        inst = packet->data[1];
        data = packet->data[2];
    }

    printf("Loco:%u ",
           address);

    /*
     * 128 Step Speed Command
     *
     * 3F xx
     *
     * bit7
     *   1 = Forward
     *   0 = Reverse
     *
     * bit0-6
     *   Speed
     */
    if(inst == 0x3F)
    {
        printf("[128SPD] ");

        printf("%s ",
               (data & 0x80)
               ? "FWD"
               : "REV");

        printf("%u",
               data & 0x7F);
    }

    /*
     * Function Group 1
     *
     * F0～F4
     */
    else if((inst & 0xE0) == 0x80)
    {
        printf("[F0-F4 ] ");

        printf("%c",
               (inst & 0x10) ? '*' : '_');

        printf("%c",
               (inst & 0x01) ? '*' : '_');

        printf("%c",
               (inst & 0x02) ? '*' : '_');

        printf("%c",
               (inst & 0x04) ? '*' : '_');

        printf("%c",
               (inst & 0x08) ? '*' : '_');
    }

    /*
     * Function Group 2
     *
     * F5～F8
     */
    else if((inst & 0xF0) == 0xB0)
    {
        printf("[F5-F8 ] ");

        print_function_bits(
            inst,
            4);
    }

    /*
     * Function Group 3
     *
     * F9～F12
     */
    else if((inst & 0xF0) == 0xA0)
    {
        printf("[F9-F12] ");

        print_function_bits(
            inst,
            4);
    }

    /*
     * Function Expansion Instruction
     *
     * F13～F68
     *
     * NMRA拡張Function命令。
     */
    else if(inst == 0xDE ||
            inst == 0xDF ||
           (inst >= 0xD8 &&
            inst <= 0xDC))
    {
        uint8_t start_f;

        /*
         * 命令コードから
         * Function開始番号を求める。
         */
        start_f =
            (inst == 0xDE)
            ? 13
            : (inst == 0xDF)
                ? 21
                : 29 +
                  (inst - 0xD8) * 8;

        printf("[F%u-F%u] ",
               start_f,
               start_f + 7);

        print_function_bits(
            data,
            8);
    }

    /*
     * 未解析コマンド
     *
     * 今後対応のため
     * 命令コードは残しておく。
     */
    else
    {
        printf("CMD=%02X",
               inst);
    }
}