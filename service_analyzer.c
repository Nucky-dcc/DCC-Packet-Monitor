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
 * @file service_analyzer.c
 * @brief Service Mode パケット解析
 *
 * Service Mode Programming Packet を解析し、
 * CVアクセス内容を表示する。
 *
 * 本モジュールは以下を扱う。
 *
 * - Direct Byte Mode
 * - Direct Bit Mode
 * - Paged Mode
 *
 * Programming Track 上で送信される
 * CV読み書きパケットの解析を目的とする。
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "service_analyzer.h"

/**
 * @brief 現在のPaged Modeページ番号
 *
 * Paged Modeではページレジスタ設定と
 * CVアクセスが別パケットで送信される。
 *
 * そのため直前に設定されたページ番号を
 * 保持しておく必要がある。
 */
static uint8_t current_page = 1;

/**
 * @brief Service Modeパケット判定
 *
 * NMRA Service Modeでは
 * 命令バイト上位4bitが 0x7 となる。
 *
 * @param packet 判定対象
 *
 * @retval true  Service Mode
 * @retval false その他
 */
bool is_service_packet(
    const DCC_PACKET *packet)
{
    if(packet == NULL)
    {
        return false;
    }

    if(packet->length < 3)
    {
        return false;
    }

    return ((packet->data[0] & 0xF0) == 0x70);
}

/**
 * @brief Service Modeパケット解析
 *
 * Direct Mode および Paged Mode を解析し、
 * 読みやすい形式で表示する。
 *
 * @param packet 解析対象
 */
void analyze_service(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    inst = packet->data[0];

    printf("Prog ");

    /*--------------------------------------------------
     * Direct Mode
     *
     * 4バイト以上のパケットは
     * Direct Programming Packet として扱う。
     *-------------------------------------------------*/
    if(packet->length >= 4)
    {
        uint16_t cv;

        /*
         * CV番号復元
         *
         * CV番号は0基準で格納されるため、
         * 表示時は +1 する。
         */
        cv =
            (((uint16_t)(inst & 0x03) << 8)
              | packet->data[1])
            + 1;

        /*
         * Direct Byte Mode
         */
        if((inst & 0xF4) == 0x74)
        {
            printf("Direct Byte CV%u",
                   cv);

            printf(
                (inst & 0x08)
                ? " Write="
                : " Verify=");

            printf("%u",
                   packet->data[2]);

            return;
        }

        /*
         * Direct Bit Mode
         */
        else if((inst & 0xFC) == 0x78)
        {
            uint8_t bit_pos;
            uint8_t bit_val;

            bit_pos =
                packet->data[2] & 0x07;

            bit_val =
                (packet->data[2] >> 3) & 0x01;

            printf("Direct Bit CV%u",
                   cv);

            printf(
                (packet->data[2] & 0x10)
                ? " Write "
                : " Verify ");

            printf("Bit%u=%u",
                   bit_pos,
                   bit_val);

            return;
        }

        printf("Direct Unknown");

        return;
    }

    /*--------------------------------------------------
     * Paged Mode
     *
     * 古いサービスモード方式。
     *
     * ページレジスタ設定後、
     * レジスタ番号でCVアクセスする。
     *-------------------------------------------------*/
    if(packet->length == 3)
    {
        uint8_t reg;

        reg = inst & 0x07;

        /*
         * Page Register設定
         */
        if(reg == 5)
        {
            current_page =
                packet->data[1];

            printf(
                "PageReg=%u",
                current_page);

            return;
        }

        {
            uint16_t cv = 0;

            /*
             * Paged Mode CV変換
             *
             * Register1～4 は
             * Page番号からCVを算出する。
             */
            if(reg <= 3)
            {
                cv =
                    ((uint16_t)
                     (current_page - 1) * 4)
                    + reg + 1;
            }

            /*
             * Special Register
             *
             * NMRA定義済みCVへ対応付け。
             */
            else if(reg == 4)
            {
                cv = 29;
            }
            else if(reg == 6)
            {
                cv = 7;
            }
            else if(reg == 7)
            {
                cv = 8;
            }

            if(cv > 0)
            {
                printf("Paged CV%u",
                       cv);

                printf(
                    (inst & 0x08)
                    ? " Write="
                    : " Verify=");

                printf("%u",
                       packet->data[1]);
            }
            else
            {
                /*
                 * Address Mode
                 *
                 * このモニタでは
                 * CV1アクセスとして表示する。
                 */
                printf("AddrMode CV1 ");

                printf(
                    (inst & 0x08)
                    ? "Write="
                    : "Verify=");

                printf("%u",
                       packet->data[1]);
            }
        }
    }
}