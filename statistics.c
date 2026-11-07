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
 * @file statistics.c
 * @brief DCC統計情報管理
 *
 * 本モジュールは DCC Monitor の統計情報を管理する。
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
 * 統計情報は menu.c の統計画面から参照される。
 */

#include <stdint.h>
#include <limits.h>

#include "statistics.h"
#include "monitor_config.h"

/**
 * @brief カウンタ上限値
 *
 * 全カウンタは飽和カウンタ方式を採用する。
 *
 * オーバーフローによる統計破壊を防ぐため、
 * UINT32_MAX到達後は増加を停止する。
 */
#define STAT_COUNTER_MAX UINT32_MAX

/*----------------------------------------------------------------------
 * グローバル統計情報
 *----------------------------------------------------------------------*/

/**
 * @brief パケット統計
 *
 * packet_analyzer が更新する。
 */
PACKET_STATISTICS packet_statistics;

/**
 * @brief 信号統計
 *
 * dcc_decode が更新する。
 */
SIGNAL_STATISTICS signal_statistics;

/*----------------------------------------------------------------------
 * 内部関数
 *----------------------------------------------------------------------*/

/**
 * @brief 飽和カウンタ加算
 *
 * UINT32_MAX到達後は加算しない。
 *
 * 統計用途では桁あふれよりも
 * 飽和の方が望ましい。
 *
 * @param counter 対象カウンタ
 */
static void count_up(
    uint32_t *counter)
{
    if(counter == NULL)
    {
        return;
    }

    if(*counter < STAT_COUNTER_MAX)
    {
        (*counter)++;
    }
}

/*----------------------------------------------------------------------
 * 初期化
 *----------------------------------------------------------------------*/

/**
 * @brief 統計初期化
 *
 * 全統計情報を初期状態へ戻す。
 *
 * システム起動時および
 * statistics_reset() から呼ばれる。
 */
void statistics_init(void)
{
    uint16_t i;

    /*--------------------------------------------------
     * パケット統計
     *-------------------------------------------------*/

    packet_statistics.total = 0;
    packet_statistics.reset = 0;
    packet_statistics.idle = 0;
    packet_statistics.loco = 0;
    packet_statistics.accessory = 0;
    packet_statistics.ops = 0;
    packet_statistics.service = 0;
    packet_statistics.crc_error = 0;

    /*
     * 累積計測時間
     */
    packet_statistics.elapsed_us = 0;

    /*--------------------------------------------------
     * 信号統計
     *-------------------------------------------------*/

    signal_statistics.bit0_count = 0;
    signal_statistics.bit1_count = 0;

    signal_statistics.bit0_total_us = 0;
    signal_statistics.bit1_total_us = 0;

    signal_statistics.range_301_10000us_count = 0;
    signal_statistics.over_10000us_count = 0;

    /*--------------------------------------------------
     * ヒストグラム初期化
     *-------------------------------------------------*/

    for(i = 0;
        i < HISTOGRAM_BUCKET_COUNT;
        i++)
    {
        signal_statistics.hist_0_300[i] = 0;
    }
}

/**
 * @brief 統計クリア
 *
 * ユーザーが統計画面で
 *
 * Rキー
 *
 * を押した際に呼ばれる。
 *
 * 実装は初期化処理へ委譲する。
 */
void statistics_reset(void)
{
    statistics_init();
}

/*----------------------------------------------------------------------
 * パケット統計
 *----------------------------------------------------------------------*/

/**
 * @brief 総受信パケット数加算
 *
 * packet_analyzer() が
 * パケット受信ごとに呼び出す。
 *
 * フィルタによる表示有無に関係なく、
 * 実際に受信した全パケットを対象とする。
 */
void statistics_count_total(void)
{
    count_up(
        &packet_statistics.total);
}

/**
 * @brief Reset Packet数加算
 *
 * NMRA Reset Packet受信時に呼び出す。
 */
void statistics_count_reset(void)
{
    count_up(
        &packet_statistics.reset);
}

/**
 * @brief Idle Packet数加算
 *
 * NMRA Idle Packet受信時に呼び出す。
 */
void statistics_count_idle(void)
{
    count_up(
        &packet_statistics.idle);
}

/**
 * @brief Loco Packet数加算
 *
 * 機関車制御パケット受信時に呼び出す。
 */
void statistics_count_loco(void)
{
    count_up(
        &packet_statistics.loco);
}

/**
 * @brief Accessory Packet数加算
 *
 * アクセサリ制御パケット受信時に呼び出す。
 */
void statistics_count_accessory(void)
{
    count_up(
        &packet_statistics.accessory);
}

/**
 * @brief OPS Packet数加算
 *
 * Main Track Programming
 * (OPS Mode)パケット受信時に呼び出す。
 */
void statistics_count_ops(void)
{
    count_up(
        &packet_statistics.ops);
}

/**
 * @brief Service Packet数加算
 *
 * Programming Track上の
 * Service Modeパケット受信時に呼び出す。
 */
void statistics_count_service(void)
{
    count_up(
        &packet_statistics.service);
}

/**
 * @brief CRC Error数加算
 *
 * CRC検査失敗時に呼び出す。
 *
 * CRCエラーも受信実績として記録し、
 * 信号品質評価に利用する。
 */
void statistics_count_crc_error(void)
{
    count_up(
        &packet_statistics.crc_error);
}

/**
 * @brief 累積受信時間加算
 *
 * パケット間隔(us)を累積する。
 *
 * packet_analyzer() が計算した
 * 前回パケットからの経過時間を加算する。
 *
 * 本プロジェクトでは
 * 「正常パケット基準」ではなく
 * 「受信した全パケット基準」
 * の時間軸を採用している。
 *
 * そのためCRCエラーや
 * フィルタ除外パケットも
 * この時間へ含まれる。
 *
 * @param elapsed_us 経過時間(us)
 */
void statistics_add_elapsed_us(
    uint32_t elapsed_us)
{
    /*
     * 64bit飽和加算
     *
     * 長時間運転時の
     * オーバーフローを防止する。
     */
    if((UINT64_MAX -
        packet_statistics.elapsed_us)
       >= elapsed_us)
    {
        packet_statistics.elapsed_us +=
            elapsed_us;
    }
    else
    {
        packet_statistics.elapsed_us =
            UINT64_MAX;
    }
}

/*----------------------------------------------------------------------
 * 信号統計
 *----------------------------------------------------------------------*/

/**
 * @brief Bit0受信数加算
 *
 * dcc_decode が
 * DCC Bit0 を検出した際に呼び出す。
 */
void statistics_count_bit0(void)
{
    count_up(
        &signal_statistics.bit0_count);
}

/**
 * @brief Bit1受信数加算
 *
 * dcc_decode が
 * DCC Bit1 を検出した際に呼び出す。
 */
void statistics_count_bit1(void)
{
    count_up(
        &signal_statistics.bit1_count);
}

/**
 * @brief Bit0パルス幅加算
 *
 * Bit0として判定されたパルス幅を
 * 累積加算する。
 *
 * 現在は主に平均値計算や
 * 将来の解析拡張用データとして保持している。
 *
 * @param width_us パルス幅(us)
 */
void statistics_add_bit0_width(
    uint32_t width_us)
{
    /*
     * 64bit飽和加算
     */
    if((UINT64_MAX -
        signal_statistics.bit0_total_us)
       >= width_us)
    {
        signal_statistics.bit0_total_us +=
            width_us;
    }
    else
    {
        signal_statistics.bit0_total_us =
            UINT64_MAX;
    }
}

/**
 * @brief Bit1パルス幅加算
 *
 * Bit1として判定されたパルス幅を
 * 累積加算する。
 *
 * @param width_us パルス幅(us)
 */
void statistics_add_bit1_width(
    uint32_t width_us)
{
    /*
     * 64bit飽和加算
     */
    if((UINT64_MAX -
        signal_statistics.bit1_total_us)
       >= width_us)
    {
        signal_statistics.bit1_total_us +=
            width_us;
    }
    else
    {
        signal_statistics.bit1_total_us =
            UINT64_MAX;
    }
}

/**
 * @brief パルス幅ヒストグラム更新
 *
 * パルス幅分布を記録する。
 *
 * 分類：
 *
 * 0～300us
 *   ヒストグラムへ記録
 *
 * 301～10000us
 *   Long Pulseとして記録
 *
 * 10000us超
 *   Signal Lossとして記録
 *
 * ヒストグラムは1us単位で集計されるため、
 * DCC信号品質やしきい値調整の評価に利用できる。
 *
 * @param width_us パルス幅(us)
 */
void statistics_add_histogram(
    uint32_t width_us)
{
    /*
     * 通常ヒストグラム領域
     *
     * index = パルス幅(us)
     */
    if(width_us <= HISTOGRAM_MAX_US)
    {
        count_up(
            &signal_statistics.
                hist_0_300[
                    width_us]);

        return;
    }

    /*
     * Long Pulse領域
     *
     * DCC Bit0/Bit1として扱わないが、
     * 異常傾向解析のため件数を保持する。
     */
    if(width_us <= LONG_PULSE_MAX_US)
    {
        count_up(
            &signal_statistics.
                range_301_10000us_count);

        return;
    }

    /*
     * 10000us超
     *
     * 信号断や入力停止など、
     * 異常状態の検出用。
     */
    count_up(
        &signal_statistics.
            over_10000us_count);
}

/*----------------------------------------------------------------------
 * 統計解析
 *----------------------------------------------------------------------*/

/**
 * @brief Bit1 Mode値取得
 *
 * ヒストグラムから
 * 最も出現回数の多いBit1幅を取得する。
 *
 * 平均値ではなくMode値を採用することで、
 * ノイズの影響を受けにくい代表値が得られる。
 *
 * 探索範囲は
 *
 * BIT1_RANGE_MIN_US
 *      ～
 * monitor_config.threshold_us
 *
 * とする。
 *
 * @return Bit1 Mode値(us)
 */
uint32_t statistics_get_bit1_mode(void)
{
    uint32_t max_count = 0;
    uint32_t mode_us = 0;
    uint32_t width;

    /*
     * Bit1領域のみ探索する。
     */
    for(width = BIT1_MODE_SEARCH_MIN_US;
        width <= monitor_config.threshold_us;
        width++)
    {
        uint32_t count;

        count =
            signal_statistics.
                hist_0_300[
                    width];

        if(count > max_count)
        {
            max_count = count;
            mode_us = width;
        }
    }

    return mode_us;
}

/**
 * @brief Bit0 Mode値取得
 *
 * ヒストグラムから
 * 最も出現回数の多いBit0幅を取得する。
 *
 * threshold_us を境界として
 * Bit0領域のみ探索する。
 *
 * @return Bit0 Mode値(us)
 */
uint32_t statistics_get_bit0_mode(void)
{
    uint32_t max_count = 0;
    uint32_t mode_us = 0;
    uint32_t width;

    /*
     * Bit0領域のみ探索する。
     */
    for(width =
            monitor_config.threshold_us + 1;
        width <= HISTOGRAM_MAX_US;
        width++)
    {
        uint32_t count;

        count =
            signal_statistics.
                hist_0_300[
                    width];

        if(count > max_count)
        {
            max_count = count;
            mode_us = width;
        }
    }

    return mode_us;
}

/**
 * @brief 指定範囲の発生回数取得
 *
 * ヒストグラムから
 * 指定範囲の総発生回数を集計する。
 *
 * 使用例
 *
 * Short Pulse
 *     0～54us
 *
 * Bit1領域
 *     55～threshold_us
 *
 * Bit0領域
 *     threshold_us+1～300us
 *
 * @param start_us 集計開始(us)
 * @param end_us   集計終了(us)
 *
 * @return 発生回数
 */
uint32_t statistics_get_range_count(
    uint32_t start_us,
    uint32_t end_us)
{
    uint32_t count = 0;
    uint32_t width;

    /*
     * 開始位置が範囲外。
     */
    if(start_us > HISTOGRAM_MAX_US)
    {
        return 0;
    }

    /*
     * 終了位置は
     * ヒストグラム最大値へ制限する。
     */
    if(end_us > HISTOGRAM_MAX_US)
    {
        end_us = HISTOGRAM_MAX_US;
    }

    /*
     * 指定範囲を積算。
     */
    for(width = start_us;
        width <= end_us;
        width++)
    {
        count +=
            signal_statistics.
                hist_0_300[
                    width];
    }

    return count;
}