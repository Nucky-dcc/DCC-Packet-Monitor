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
 * @file service_analyzer.h
 * @brief Service Mode パケット解析
 *
 * サービスモードプログラミングパケットを解析し、
 * 人が読める形式へ変換する。
 *
 * 対応モード
 *
 * - Direct Byte Mode
 * - Direct Bit Mode
 * - Paged Mode
 *
 * 主にプログラミングトラック上で使用される
 * CV読み書きパケットの解析を担当する。
 */

#ifndef SERVICE_ANALYZER_H
#define SERVICE_ANALYZER_H

#include <stdbool.h>

#include "dcc_packet.h"

/**
 * @brief Service Modeパケット判定
 *
 * 指定されたDCCパケットが
 * Service Modeパケットか判定する。
 *
 * @param packet 判定対象パケット
 *
 * @retval true  Service Mode
 * @retval false その他
 */
bool is_service_packet(
    const DCC_PACKET *packet);

/**
 * @brief Service Modeパケット解析
 *
 * パケット内容を解析し、
 * 人が理解しやすいテキスト形式で表示する。
 *
 * @param packet 解析対象パケット
 */
void analyze_service(
    const DCC_PACKET *packet);

#endif