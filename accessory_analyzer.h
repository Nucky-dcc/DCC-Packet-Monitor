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
 * @file accessory_analyzer.h
 * @brief アクセサリデコーダパケット解析
 *
 * NMRA Basic Accessory Decoder Packet を解析し、
 * 人が読みやすい表示へ変換する。
 *
 * 主な機能
 *
 * - アクセサリアドレス復元
 * - 出力番号変換
 * - 分岐方向表示
 *
 * ポイントマシンや信号機などの
 * アクセサリ機器の制御解析を目的とする。
 */

#ifndef ACCESSORY_ANALYZER_H
#define ACCESSORY_ANALYZER_H

#include "dcc_packet.h"

/**
 * @brief Accessory Packet解析
 *
 * Basic Accessory Decoder Packet を解析し、
 * アドレスおよび動作方向を表示する。
 *
 * @param packet 解析対象パケット
 */
void analyze_accessory(
    const DCC_PACKET *packet);

#endif