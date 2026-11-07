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
 * @file dcc_packet.c
 * @brief DCCパケット復元処理
 *
 * dcc_decode が出力する DCC Bit列から
 * DCCパケットを復元する。
 *
 * 処理の流れ
 *
 * DCC Bit
 * ↓
 * プリアンブル検出
 * ↓
 * 開始ビット検出
 * ↓
 * バイト復元
 * ↓
 * セパレータ判定
 * ↓
 * パケット完成
 *
 * CRC判定は行わない。
 * CRCチェックは packet_analyzer が担当する。
 */

#include <string.h>

#include "dcc_decode.h"
#include "dcc_packet.h"
#include "monitor_config.h"

/**
 * @brief パケット復元状態
 */
typedef enum
{
    /**
     * プリアンブル受信中
     */
    PACKET_STATE_PREAMBLE,

    /**
     * データバイト受信中
     */
    PACKET_STATE_BYTE,

    /**
     * セパレータ判定中
     */
    PACKET_STATE_SEPARATOR

} PACKET_STATE;

/**
 * @brief 復元状態
 */
static PACKET_STATE state;

/**
 * @brief 連続Bit1数
 *
 * プリアンブル判定に使用する。
 */
static uint16_t preamble_count;

/**
 * @brief 現在構築中のバイト
 */
static uint8_t current_byte;

/**
 * @brief 現在のビット数
 */
static uint8_t bit_count;

/**
 * @brief 構築中パケット
 */
static DCC_PACKET current_packet;

/**
 * @brief 完成済みパケットバッファ
 */
static DCC_PACKET packet_buffer[
    DCC_PACKET_BUFFER_SIZE];

/**
 * @brief 書き込み位置
 */
static volatile uint16_t packet_head;

/**
 * @brief 読み出し位置
 */
static volatile uint16_t packet_tail;

/**
 * @brief パケット完成処理
 *
 * current_packet を
 * 完成済みバッファへ登録する。
 */
static void packet_complete(void)
{
    uint16_t next_head;

    /*
     * 長さ0は無効。
     */
    if(current_packet.length == 0)
    {
        return;
    }

    next_head =
        (packet_head + 1)
        % DCC_PACKET_BUFFER_SIZE;

    /*
     * バッファ満杯時は
     * 新規パケットを破棄する。
     */
    if(next_head == packet_tail)
    {
        return;
    }

    packet_buffer[packet_head] =
        current_packet;

    packet_head = next_head;
}

/**
 * @brief パケット復元初期化
 */
void dcc_packet_init(void)
{
    memset(packet_buffer,
           0,
           sizeof(packet_buffer));

    memset(&current_packet,
           0,
           sizeof(current_packet));

    packet_head = 0;
    packet_tail = 0;

    state = PACKET_STATE_PREAMBLE;

    preamble_count = 0;

    bit_count = 0;
    current_byte = 0;
}

/**
 * @brief DCCパケット復元処理
 *
 * Bit列からDCCパケットを再構築する。
 */
void dcc_packet_task(void)
{
    DCC_BIT bit;

    while(dcc_get_bit(&bit))
    {
        switch(state)
        {
            case PACKET_STATE_PREAMBLE:

                /*
                 * プリアンブルは
                 * 連続したBit1。
                 */
                if(bit.bit)
                {
                    if(preamble_count < 255)
                    {
                        preamble_count++;
                    }
                }
                else
                {
                    /*
                     * プリアンブル終了。
                     *
                     * 規定数以上のBit1後に
                     * Bit0が来たら
                     * パケット開始とみなす。
                     */
                    if(preamble_count >=
                       monitor_config.preamble_count)
                    {
                        memset(&current_packet,
                               0,
                               sizeof(current_packet));

                        current_packet.timestamp_us =
                            bit.timestamp_us;

                        bit_count = 0;
                        current_byte = 0;

                        state =
                            PACKET_STATE_BYTE;
                    }

                    preamble_count = 0;
                }

                break;

            case PACKET_STATE_BYTE:

                /*
                 * MSB Firstで
                 * データを構築する。
                 */
                current_byte =
                    (current_byte << 1)
                    | bit.bit;

                bit_count++;

                /*
                 * 8bit揃った。
                 */
                if(bit_count >= 8)
                {
                    if(current_packet.length <
                       DCC_PACKET_MAX_BYTES)
                    {
                        current_packet.data[
                            current_packet.length
                        ] = current_byte;

                        current_packet.length++;
                    }

                    bit_count = 0;
                    current_byte = 0;

                    state =
                        PACKET_STATE_SEPARATOR;
                }

                break;

            case PACKET_STATE_SEPARATOR:

                /*
                 * DCCセパレータ
                 *
                 * 0 : 次バイトあり
                 * 1 : パケット終了
                 */
                if(bit.bit == 0)
                {
                    state =
                        PACKET_STATE_BYTE;
                }
                else
                {
                    /*
                     * パケット完成。
                     */
                    packet_complete();

                    /*
                     * 次のプリアンブルへ。
                     *
                     * 直前のBit1は既に
                     * 次パケットの
                     * プリアンブル1bit目でもある。
                     */
                    state =
                        PACKET_STATE_PREAMBLE;

                    preamble_count = 1;
                }

                break;

            default:

                /*
                 * 状態異常時は復帰。
                 */
                state =
                    PACKET_STATE_PREAMBLE;

                preamble_count = 0;

                break;
        }
    }
}

/**
 * @brief パケット取得
 *
 * 完成済みパケットをFIFOで取得する。
 */
bool dcc_get_packet(
    DCC_PACKET *packet)
{
    if(packet_head == packet_tail)
    {
        return false;
    }

    *packet =
        packet_buffer[packet_tail];

    packet_tail =
        (packet_tail + 1)
        % DCC_PACKET_BUFFER_SIZE;

    return true;
}

/**
 * @brief パケットバッファ破棄
 *
 * 未処理パケットを全て破棄する。
 */
void dcc_packet_flush(void)
{
    packet_tail = packet_head;
}
