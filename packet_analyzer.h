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
 * @file packet_analyzer.h
 * @brief DCCパケット解析および表示制御
 *
 * dcc_packet が復元した DCC_PACKET を解析し、
 * テキスト表示および統計更新を行う。
 *
 * 本モジュールは DCC Packet Monitor の中心となる
 * パケット解析レイヤであり、
 * パケット内容の詳細解析を各 analyzer モジュールへ
 * 振り分ける責務を持つ。
 *
 * 主な機能
 *
 * - 通番管理
 * - パケット間隔計測
 * - CRC検査
 * - パケット種別判定
 * - 各種フィルタ処理
 * - 統計情報更新
 * - モニタ表示
 *
 * 解析対象となるパケットは
 * dcc_packet モジュールにより CRC バイトまで含めて
 * 復元済みであることを前提とする。
 */

#ifndef PACKET_ANALYZER_H
#define PACKET_ANALYZER_H

#include "dcc_packet.h"

/**
 * @brief パケット解析状態初期化
 *
 * 以下の内部状態を初期化する。
 *
 * - パケット通番
 * - パケット間隔計測基準時刻
 *
 * MENU_MONITOR開始時に呼び出され、
 * 毎回同じ条件で測定をスタートできるようにする。
 */
void packet_analyzer_init(void);

/**
 * @brief DCCパケット解析
 *
 * 受信した DCC_PACKET を解析する。
 *
 * 実行内容
 *
 * - CRC確認
 * - パケット種別判定
 * - フィルタ適用
 * - 統計更新
 * - 表示出力
 *
 * CRCエラーであっても通番および時間計測は継続する。
 * これにより監視結果の時間軸が一貫するよう設計している。
 *
 * 種別判定後は必要に応じて
 *
 * - loco_analyzer
 * - accessory_analyzer
 * - service_analyzer
 * - ops_analyzer
 *
 * を呼び出す。
 *
 * @param packet 解析対象パケット
 */
void packet_analyzer(
    const DCC_PACKET *packet);

#endif
