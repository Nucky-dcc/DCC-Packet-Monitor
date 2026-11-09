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
 * @file monitor_config.h
 * @brief DCC Packet Monitor 設定管理
 *
 * ユーザーがメニューから変更可能な
 * 各種設定値を保持する。
 *
 * 本モジュールは設定値の保持のみを担当し、
 * 解析処理や表示処理は行わない。
 */

#ifndef MONITOR_CONFIG_H
#define MONITOR_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief パケット表示フィルタ
 *
 * モニタ画面で表示する
 * パケット種別を指定する。
 */
typedef enum
{
    FILTER_ALL,

    /**
     * @brief 機関車パケット全体
     */
    FILTER_LOCO,

    /**
     * @brief 機関車速度制御のみ
     */
    FILTER_LOCO_SPEED,

    /**
     * @brief 機関車ファンクションのみ
     */
    FILTER_LOCO_FUNCTION,

    /**
     * @brief アクセサリパケットのみ
     */
    FILTER_ACCESSORY,

    /**
     * @brief OPS Mode Programmingのみ
     */
    FILTER_OPS

} FILTER_MODE;

/**
 * @brief モニタ設定
 *
 * メニューから変更可能な
 * ユーザー設定値を保持する。
 *
 * DCC解析ロジックは本構造体を参照して
 * フィルタ処理や表示制御を行う。
 */
typedef struct
{
    /**
     * @brief パケット種別フィルタ
     *
     * モニタ画面に表示する
     * パケット種別を指定する。
     */
    FILTER_MODE filter;

    /**
     * @brief RESETパケット表示許可
     *
     * false の場合、
     * RESETパケットは表示されない。
     */
    bool show_reset;

    /**
     * @brief IDLEパケット表示許可
     *
     * false の場合、
     * IDLEパケットは表示されない。
     */
    bool show_idle;

    /**
     * @brief アドレスフィルタ
     *
     * 0 は無効を意味する。
     *
     * 0以外の場合、
     * 指定アドレスのパケットのみ表示する。
     */
    uint16_t address_filter;

    /**
     * @brief DCC Bit判定閾値[us]
     *
     * dcc_decode モジュールにおいて、
     * パルス幅がこの値以下なら
     * Bit1 と判定する。
     *
     * コマンドステーションごとの
     * 波形差異を評価できるよう
     * ユーザー変更可能としている。
     */
    uint16_t threshold_us;

    /**
     * @brief プリアンブル判定数
     *
     * dcc_packet モジュールが
     * パケット開始とみなすために必要な
     * 連続Bit1数。
     *
     * DCC信号品質評価や
     * 実験用途のため変更可能としている。
     */
    uint8_t preamble_count;

} MONITOR_CONFIG;

/**
 * @brief モニタ設定
 *
 * システム全体で参照される
 * 設定情報。
 */
extern MONITOR_CONFIG monitor_config;

/**
 * @brief 設定初期化
 *
 * すべての設定値を
 * デフォルト状態へ初期化する。
 *
 * システム起動時に一度だけ実行する。
 */
void monitor_config_init(void);

#endif /* MONITOR_CONFIG_H */