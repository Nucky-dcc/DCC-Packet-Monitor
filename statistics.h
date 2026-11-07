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
 * @file statistics.h
 * @brief DCC統計情報管理
 *
 * 本モジュールは DCC Packet Monitor における
 * 各種統計情報の管理を担当する。
 *
 * 主な機能
 *
 * - パケット統計
 * - CRCエラー統計
 * - Bit0/Bit1統計
 * - パルス幅ヒストグラム
 * - Mode値算出
 * - 範囲別集計
 *
 * 統計情報はグローバル構造体として保持され、
 * menu.c の統計画面から参照される。
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>

/*----------------------------------------------------------------------
 * Histogram定義
 *----------------------------------------------------------------------*/

/**
 * @brief ヒストグラム最小値
 */
#define HISTOGRAM_MIN_US              0U

/**
 * @brief Short Pulse最大値
 *
 * DCC信号としては異常に短い領域。
 */
#define SHORT_PULSE_MAX_US           54U

/**
 * @brief Bit1領域最小値
 */
#define BIT1_RANGE_MIN_US            55U

/**
 * @brief ヒストグラム最大値
 *
 * 0～300usを1us単位で集計する。
 */
#define HISTOGRAM_MAX_US            300U

/**
 * @brief Long Pulse最小値
 */
#define LONG_PULSE_MIN_US          301U

/**
 * @brief Long Pulse最大値
 */
#define LONG_PULSE_MAX_US        10000U

/**
 * @brief ヒストグラム配列要素数
 *
 * 0～300us
 */
#define HISTOGRAM_BUCKET_COUNT \
    (HISTOGRAM_MAX_US + 1U)

/**
 * @brief Bit1 Mode探索開始位置
 */
#define BIT1_MODE_SEARCH_MIN_US \
    BIT1_RANGE_MIN_US

/**
 * @brief Bit0 Mode探索開始位置
 *
 * DCCのBit0は通常100us以上であるため
 * 探索開始位置を100usとしている。
 */
#define BIT0_MODE_SEARCH_MIN_US \
    100U

/*----------------------------------------------------------------------
 * パケット統計
 *----------------------------------------------------------------------*/

/**
 * @brief パケット統計情報
 *
 * packet_analyzer が更新する。
 */
typedef struct
{
    /**
     * 総受信パケット数
     */
    uint32_t total;

    /**
     * Reset Packet数
     */
    uint32_t reset;

    /**
     * Idle Packet数
     */
    uint32_t idle;

    /**
     * Loco Packet数
     */
    uint32_t loco;

    /**
     * Accessory Packet数
     */
    uint32_t accessory;

    /**
     * OPS Packet数
     */
    uint32_t ops;

    /**
     * Service Packet数
     */
    uint32_t service;

    /**
     * CRC Error数
     */
    uint32_t crc_error;

    /**
     * 累積受信時間
     *
     * 単位: us
     */
    uint64_t elapsed_us;

} PACKET_STATISTICS;

/*----------------------------------------------------------------------
 * 信号統計
 *----------------------------------------------------------------------*/

/**
 * @brief DCC信号統計情報
 *
 * dcc_decode モジュールが更新する。
 */
typedef struct
{
    /**
     * Bit0受信数
     */
    uint32_t bit0_count;

    /**
     * Bit1受信数
     */
    uint32_t bit1_count;

    /**
     * Bit0累積パルス幅
     */
    uint64_t bit0_total_us;

    /**
     * Bit1累積パルス幅
     */
    uint64_t bit1_total_us;

    /**
     * パルス幅ヒストグラム
     *
     * index = パルス幅(us)
     *
     * 例:
     *
     * hist_0_300[58]
     *
     * は、
     * 58us のパルス発生回数。
     */
    uint32_t hist_0_300[
        HISTOGRAM_BUCKET_COUNT];

    /**
     * 301～10000us発生数
     */
    uint32_t range_301_10000us_count;

    /**
     * 10000us超発生数
     *
     * 信号断や入力異常検出用。
     */
    uint32_t over_10000us_count;

} SIGNAL_STATISTICS;

/*----------------------------------------------------------------------
 * グローバル統計情報
 *----------------------------------------------------------------------*/

/**
 * @brief パケット統計
 */
extern PACKET_STATISTICS
    packet_statistics;

/**
 * @brief 信号統計
 */
extern SIGNAL_STATISTICS
    signal_statistics;

/*----------------------------------------------------------------------
 * 初期化
 *----------------------------------------------------------------------*/

/**
 * @brief 統計初期化
 */
void statistics_init(void);

/**
 * @brief 統計クリア
 *
 * 全統計情報を初期状態へ戻す。
 */
void statistics_reset(void);

/*----------------------------------------------------------------------
 * パケット統計
 *----------------------------------------------------------------------*/

void statistics_count_total(void);
void statistics_count_reset(void);
void statistics_count_idle(void);
void statistics_count_loco(void);
void statistics_count_accessory(void);
void statistics_count_ops(void);
void statistics_count_service(void);
void statistics_count_crc_error(void);

void statistics_add_elapsed_us(
    uint32_t elapsed_us);

/*----------------------------------------------------------------------
 * 信号統計
 *----------------------------------------------------------------------*/

void statistics_count_bit0(void);
void statistics_count_bit1(void);

void statistics_add_bit0_width(
    uint32_t width_us);

void statistics_add_bit1_width(
    uint32_t width_us);

void statistics_add_histogram(
    uint32_t width_us);

/*----------------------------------------------------------------------
 * 統計解析
 *----------------------------------------------------------------------*/

/**
 * @brief Bit1 Mode値取得
 *
 * ヒストグラム中で最も発生回数が多い
 * Bit1パルス幅を返す。
 *
 * @return Mode値(us)
 */
uint32_t statistics_get_bit1_mode(void);

/**
 * @brief Bit0 Mode値取得
 *
 * ヒストグラム中で最も発生回数が多い
 * Bit0パルス幅を返す。
 *
 * @return Mode値(us)
 */
uint32_t statistics_get_bit0_mode(void);

/**
 * @brief 指定範囲集計
 *
 * ヒストグラムから指定範囲の
 * 発生回数合計を取得する。
 *
 * @param start_us 開始値
 * @param end_us   終了値
 *
 * @return 発生回数
 */
uint32_t statistics_get_range_count(
    uint32_t start_us,
    uint32_t end_us);

#endif