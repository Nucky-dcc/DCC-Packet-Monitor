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
 * @file menu.h
 * @brief ユーザーインタフェース管理
 *
 * DCC Packet Monitor の
 * テキストベースユーザーインタフェースを管理する。
 *
 * 本モジュールは以下を担当する。
 *
 * - メニュー表示
 * - キー入力処理
 * - 設定変更
 * - 画面状態管理
 *
 * DCC解析処理は担当しない。
 */

#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

/**
 * @brief メニュー状態
 *
 * 画面遷移状態を表す。
 *
 * menu.c が状態遷移を管理し、
 * 他モジュールは必要に応じて
 * 現在状態を参照する。
 */
typedef enum
{
    /**
     * @brief メニュー未表示
     *
     * 起動直後の待機状態。
     */
    MENU_NONE,

    /**
     * @brief メインメニュー
     */
    MENU_MAIN,

    /**
     * @brief パケット監視画面
     */
    MENU_MONITOR,

    /**
     * @brief パケット統計画面
     */
    MENU_PACKET_STATS,

    /**
     * @brief DCC信号統計画面
     */
    MENU_SIGNAL_STATS,

    /**
     * @brief ヒストグラム表示画面
     */
    MENU_SIGNAL_HISTOGRAM,

    /**
     * @brief 設定メニュー
     */
    MENU_SETTINGS,

    /**
     * @brief パケット種別フィルタ設定
     */
    MENU_FILTER,

    /**
     * @brief アドレスフィルタ設定
     */
    MENU_ADDRESS,

    /**
     * @brief Bit判定閾値設定
     *
     * dcc_decode モジュールで使用する
     * Bit0/Bit1判定閾値を変更する。
     */
    MENU_THRESHOLD,

    /**
     * @brief プリアンブル数設定
     *
     * dcc_packet モジュールが
     * パケット開始と判定するための
     * 最小Bit1数を変更する。
     */
    MENU_PREAMBLE

} MENU_STATE;

/**
 * @brief メインメニュー表示
 *
 * メイン画面へ遷移し、
 * メニューを表示する。
 *
 * システム起動後、
 * ENTERキー押下時に呼び出される。
 */
void menu_show_main(void);

/**
 * @brief キー入力処理
 *
 * ユーザー入力を現在のメニュー状態に応じて
 * 解釈し、必要な画面遷移や設定変更を行う。
 *
 * @param ch 入力文字
 */
void menu_process_key(int ch);

/**
 * @brief パケット監視画面状態取得
 *
 * packet_analyzer が
 * 表示処理を行うべきか判断するために使用する。
 *
 * @retval true  パケット監視画面
 * @retval false その他の画面
 */
bool menu_is_monitor_mode(void);

#endif /* MENU_H */