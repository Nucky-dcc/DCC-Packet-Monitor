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
 * @file monitor_config.c
 * @brief DCC Packet Monitor 設定管理
 *
 * ユーザー設定値の実体および
 * 初期化処理を提供する。
 *
 * 設定値はシステム全体から参照され、
 * 表示フィルタやDCC解析条件の制御に使用される。
 */

#include "monitor_config.h"

/**
 * @brief モニタ設定情報
 *
 * システム全体で共有される設定情報。
 *
 * 起動時は monitor_config_init() により
 * デフォルト値へ初期化される。
 */
MONITOR_CONFIG monitor_config;

/**
 * @brief モニタ設定初期化
 *
 * すべての設定値をデフォルト状態へ初期化する。
 *
 * 初期値の設計方針:
 *
 * - 全パケット表示
 * - RESET表示有効
 * - IDLE表示有効
 * - アドレスフィルタ無効
 * - Bit判定閾値 80us
 * - プリアンブル判定数 14bit
 *
 * ユーザーが設定変更を行った場合でも、
 * 本関数を呼び出すことで既知の初期状態へ復帰できる。
 */
void monitor_config_init(void)
{
    /**
     * すべてのパケットを表示する。
     *
     * 初回起動時は、
     * DCC信号全体の観察を優先する。
     */
    monitor_config.filter = FILTER_ALL;

    /**
     * RESETパケット表示を有効化。
     *
     * RESETはサービスモード解析時に重要なため、
     * デフォルトでは表示する。
     */
    monitor_config.show_reset = true;

    /**
     * IDLEパケット表示を有効化。
     *
     * コマンドステーションの動作状況を把握しやすくするため、
     * デフォルトでは表示する。
     */
    monitor_config.show_idle = true;

    /**
     * アドレスフィルタ無効。
     *
     * 0をフィルタ無効値として使用する。
     */
    monitor_config.address_filter = 0;

    /**
     * DCC Bit判定閾値。
     *
     * 一般的なNMRA DCC信号を基準とした
     * 初期設定値。
     *
     * ユーザーはメニューから変更可能。
     */
    monitor_config.threshold_us = 80;

    /**
     * プリアンブル判定数。
     *
     * NMRA規格では最小10bit以上の
     * プリアンブルが規定されている。
     *
     * 本ツールでは信号品質評価や
     * 実験用途を考慮し、
     * 初期値として14bitを採用している。
     */
    monitor_config.preamble_count = 14;
}