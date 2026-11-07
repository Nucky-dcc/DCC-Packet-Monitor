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
 * @file dcc_packet.h
 * @brief DCCパケット復元処理
 *
 * dcc_decode が出力する Bit列から
 * DCCパケットを復元する。
 *
 * 主な機能
 *
 * - プリアンブル検出
 * - バイト復元
 * - パケット終端判定
 * - パケットバッファ管理
 *
 * CRCチェックは行わない。
 * CRC検証は packet_analyzer が担当する。
 */

#ifndef DCC_PACKET_H
#define DCC_PACKET_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 最大パケットサイズ
 *
 * 現在のDCC Monitorで扱う
 * 最大データ長。
 */
#define DCC_PACKET_MAX_BYTES    8

/**
 * @brief パケットバッファ数
 */
#define DCC_PACKET_BUFFER_SIZE  32

/**
 * @brief 復元済みDCCパケット
 */
typedef struct
{
    /**
     * @brief パケット受信時刻
     *
     * プリアンブル終端時刻を記録する。
     */
    uint32_t timestamp_us;

    /**
     * @brief パケットデータ
     */
    uint8_t data[DCC_PACKET_MAX_BYTES];

    /**
     * @brief データ長
     *
     * CRCバイトを含む。
     */
    uint8_t length;

} DCC_PACKET;

/**
 * @brief パケット取得
 *
 * 完成済みパケットを1件取得する。
 *
 * @param packet 取得先
 *
 * @retval true  パケットあり
 * @retval false パケットなし
 */
bool dcc_get_packet(
    DCC_PACKET *packet);

/**
 * @brief パケット復元初期化
 */
void dcc_packet_init(void);

/**
 * @brief DCCパケット復元処理
 */
void dcc_packet_task(void);

/**
 * @brief パケットバッファ破棄
 *
 * 未処理パケットを全て破棄する。
 */
void dcc_packet_flush(void);

#endif