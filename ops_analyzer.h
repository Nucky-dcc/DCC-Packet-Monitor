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
 * @file ops_analyzer.h
 * @brief OPS Mode Programming Packet解析
 *
 * Main Track Programming（OPS Mode）パケットを
 * 解析し、人が読める形式で表示する。
 *
 * 主な機能
 *
 * - OPS Packet判定
 * - アドレス取得
 * - CV番号取得
 * - 書き込みデータ取得
 *
 * 本モジュールはCV書き込みを解析するため、
 * デコーダ開発時の監視用途を想定している。
 */

#ifndef OPS_ANALYZER_H
#define OPS_ANALYZER_H

#include <stdbool.h>

#include "dcc_packet.h"

/**
 * @brief OPS Mode Packet判定
 *
 * 指定されたパケットが
 * OPS Mode Programming Packet か判定する。
 *
 * @param packet 判定対象
 *
 * @retval true  OPS Packet
 * @retval false その他
 */
bool is_ops_packet(
    const DCC_PACKET *packet);

/**
 * @brief OPS Mode Packet解析
 *
 * アドレス、CV番号および
 * 書き込みデータを表示する。
 *
 * @param packet 解析対象
 */
void analyze_ops(
    const DCC_PACKET *packet);

#endif