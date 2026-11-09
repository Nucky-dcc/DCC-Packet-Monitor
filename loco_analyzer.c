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

/*
 * NMRA S-9.2
 * 28 Step Speed Table
 *
 * index:
 *   CS3S2S1S0
 *
 * value:
 *   0   = STOP
 *   255 = E-STOP
 *   1～28 = Speed Step
 */
static const uint8_t speed28_table[32] =
{
      0,255, 1, 3, 5, 7, 9,11,
     13,15,17,19,21,23,25,27,

      0,255, 2, 4, 6, 8,10,12,
     14,16,18,20,22,24,26,28
};

/**
 * @brief Advanced Consist Control 命令表示
 *
 * NMRA S-9.2
 * Advanced Consist Control Instruction
 *
 * 本モニタでは、パケット種別のみ表示する。
 *
 * 実機検証環境が無いため、詳細解析は未実装。
 *
 * 表示例:
 *
 *   Loco:3 [ADV CONSIST]
 *
 * @param inst 命令バイト
 */
static void print_advanced_consist(
    uint8_t inst)
{
    (void)inst;

    printf("[ADV CONSIST]");
}

/**
 * @brief 14Step / 28Step 速度命令表示
 *
 * NMRA Baseline Speed Instruction
 *
 *   01DCSSSS
 *
 * を解析する。
 *
 * DCCパケットのみでは、
 *
 * ・14Step運転
 * ・28Step運転
 *
 * のどちらで送出された命令なのか
 * 判別できない。
 *
 * そのため本モニタでは、
 *
 *   [14/28SPD]
 *
 * と表示し、
 *
 *   14Step解釈
 *   28Step解釈
 *
 * の両方を同時に表示する。
 *
 * ------------------------------------------------
 * 14 Step 解釈
 * ------------------------------------------------
 *
 *   D = 方向
 *   C = F0
 *   SSSS = 速度コード
 *
 *   0000 = STOP
 *   0001 = E-STOP
 *   0010～1111 = Speed Step 1～14
 *
 * 表示例
 *
 *   FWD 5 F0=ON
 *
 * ------------------------------------------------
 * 28 Step 解釈
 * ------------------------------------------------
 *
 *   D = 方向
 *   C = 速度LSB
 *   SSSS = 速度上位4bit
 *
 * NMRA S-9.2 の速度表
 *
 *   CS3S2S1S0
 *
 * に従い、
 *
 *   STOP
 *   E-STOP
 *   Speed Step 1～28
 *
 * へ変換する。
 *
 * 例
 *
 *   00010 = Step 1
 *   10010 = Step 2
 *   00011 = Step 3
 *   10011 = Step 4
 *    ...
 *   11111 = Step 28
 *
 * NMRAとの後方互換性維持のため、
 * 14StepでF0として使用されていた
 * Cビットを、
 * 28Stepでは速度LSBとして利用する。
 *
 * ------------------------------------------------
 *
 * STOPおよびE-STOPは、
 * 14Step/28Stepの両解釈で同一のため、
 *
 *   [14/28SPD] STOP
 *
 *   [14/28SPD] E-STOP
 *
 * として表示する。
 *
 * @param inst 命令バイト
 */
static void print_speed_14_28(
    uint8_t inst)
{
    bool forward;
    bool f0;

    uint8_t code14;
    uint8_t code28;

    uint8_t step14;
    uint8_t step28;

    /*
     * 方向
     *
     * D:
     *   1 = Forward
     *   0 = Reverse
     */
    forward =
        (inst & 0x20) != 0;

    /*
     * Cビット
     *
     * 14Step:
     *   F0
     *
     * 28Step:
     *   速度LSB
     */
    f0 =
        (inst & 0x10) != 0;

    /*
     * ----- 14Stepコード -----
     *
     * Cビットは含めない。
     */
    code14 =
        inst & 0x0F;

    /*
    * NMRA S-9.2
    *
    * CS3S2S1S0
    *
    * に変換する。
    */
    code28 =
        ((inst & 0x10) ? 0x10 : 0)
        |
        (inst & 0x0F);

    /*
     * STOP
     *
     * 00000
     * 10000
     */
    if(speed28_table[code28] == 0)
    {
        printf("[14/28SPD] STOP");
        return;
    }

    /*
     * E-STOP
     *
     * 00001
     * 10001
     */
    if(speed28_table[code28] == 255)
    {
        printf("[14/28SPD] E-STOP");
        return;
    }

    /*
     * 14STEP表示値
     *
     * 2～15
     * ↓
     * 1～14
     */
    step14 =
        (code14 >= 2)
        ? (code14 - 1)
        : 0;

    /*
     * 28STEP表示値
     *
     * NMRA速度テーブルから
     * 求める。
     */
    step28 =
        speed28_table[code28];

    printf("[14/28SPD] ");

    /*
     * 14Step解釈
     */
    printf("%s %u ",
           forward
           ? "FWD"
           : "REV",
           step14);

    printf("F0=%s",
           f0
           ? "ON"
           : "OFF");

    /*
     * 28Step解釈
     */
    printf(" / ");

    printf("%s %u",
           forward
           ? "FWD"
           : "REV",
           step28);
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
    * Advanced Consist Control Instruction
    *
    * NMRA S-9.2
    *
    * 0001DCSS
    */
    if((inst & 0xF0) == 0x10)
    {
        print_advanced_consist(inst);
        return;
    }

    /*
     * 14 / 28 Step Speed Command
     *
     * 01DCSSSS
     *
     * DCCパケットのみでは
     * 14Step設定か
     * 28Step設定か
     * 判定できないため、
     * 両解釈を表示する。
     */
    else if((inst & 0xC0) == 0x40)
    {
        print_speed_14_28(
            inst);
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
     * 未対応命令
     *
     * 将来的な解析のため
     * 命令コードを残す。
     */
    else
    {
        printf("UNKNOWN INST=%02X",
               inst);
    }
}