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
 * @file loco_analyzer.h
 * @brief 機関車パケット解析
 *
 * 機関車制御パケットを解析し、
 * 人が理解しやすい形式へ表示する。
 *
 * 主な解析対象
 *
 * - Short Address
 * - Long Address
 * - 128 Step Speed
 * - Function F0～F68
 *
 * DCC Packet Monitor では、
 * packet_analyzer から呼び出される
 * 機関車パケット専用解析モジュールとして
 * 使用される。
 */

#ifndef LOCO_ANALYZER_H
#define LOCO_ANALYZER_H

#include "dcc_packet.h"

/**
 * @brief 機関車パケット解析
 *
 * パケット内容を解析し、
 * アドレス、速度、Function状態などを
 * 表示する。
 *
 * @param packet 解析対象パケット
 */
void analyze_loco(
    const DCC_PACKET *packet);

#endif