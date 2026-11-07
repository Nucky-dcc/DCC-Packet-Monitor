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
 * @file ops_analyzer.c
 * @brief OPS Mode Programming Packet解析
 *
 * Main Track Programming Packet を解析する。
 *
 * OPS Modeは運転中の車両に対して
 * 本線上でCV変更を実施する仕組みであり、
 * デコーダ開発時の解析対象として重要である。
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ops_analyzer.h"

/**
 * @brief OPS Mode Packet判定
 *
 * OPS Mode Programming Packet は
 * 命令バイト上位4bitが 1110b となる。
 *
 * アドレス長により
 * 命令バイト位置が異なるため、
 * Short Address と Long Address を
 * 分けて処理する。
 *
 * @param packet 判定対象
 *
 * @retval true  OPS Packet
 * @retval false その他
 */
bool is_ops_packet(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    if(packet == NULL)
    {
        return false;
    }

    /*
     * Short Address Packet
     *
     * Address
     * Instruction
     * CV
     * Data
     * CRC
     */
    if((packet->data[0] & 0xC0) != 0xC0)
    {
        if(packet->length < 5)
        {
            return false;
        }

        inst = packet->data[1];
    }

    /*
     * Long Address Packet
     *
     * Address High
     * Address Low
     * Instruction
     * CV
     * Data
     * CRC
     */
    else
    {
        if(packet->length < 6)
        {
            return false;
        }

        inst = packet->data[2];
    }

    /*
     * OPS Mode判定
     *
     * E0～EF
     */
    return
        ((inst & 0xF0) == 0xE0);
}

/**
 * @brief OPS Mode Packet解析
 *
 * アドレス、CV番号および
 * 書き込みデータを表示する。
 *
 * 表示例
 *
 * OPS Addr3 CV29 Write=38
 * OPS Addr100 CV1 Write=5
 *
 * @param packet 解析対象
 */
void analyze_ops(
    const DCC_PACKET *packet)
{
    uint16_t address;
    uint16_t cv;

    uint8_t inst;
    uint8_t cv_low;
    uint8_t data;

    /*
     * Short Address
     *
     * 1バイトアドレス形式。
     */
    if((packet->data[0] & 0xC0) != 0xC0)
    {
        address = packet->data[0];

        inst   = packet->data[1];
        cv_low = packet->data[2];
        data   = packet->data[3];
    }

    /*
     * Long Address
     *
     * 14bitアドレスとして復元する。
     */
    else
    {
        address =
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             | packet->data[1];

        inst   = packet->data[2];
        cv_low = packet->data[3];
        data   = packet->data[4];
    }

    /*
     * CV番号復元
     *
     * NMRAではCV番号を
     * 0基準で格納するため、
     * 表示時は +1 する。
     */
    cv =
        (((uint16_t)(inst & 0x03) << 8)
          | cv_low)
        + 1;

    /*
     * OPS Modeは
     * Main Track Programming のため、
     * 対象アドレスも表示する。
     */
    printf(
        "OPS Addr%u CV%u Write=%u",
        address,
        cv,
        data);
}