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
 * @file menu.c
 * @brief ユーザーインタフェース管理
 *
 * DCC Packet Monitor のテキストベースユーザー
 * インタフェースを実装する。
 *
 * 本モジュールの責務
 *
 * - メニュー表示
 * - キー入力処理
 * - 設定変更
 * - 画面遷移管理
 *
 * DCCパケット解析は行わず、
 * ユーザー操作および表示のみを担当する。
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "menu.h"
#include "monitor_config.h"
#include "packet_analyzer.h"
#include "dcc_packet.h"
#include "statistics.h"

/**
 * @brief 現在の画面状態
 *
 * menu_process_key() が管理する
 * UIステートマシンの状態変数。
 */
static MENU_STATE current_menu =
    MENU_NONE;

/**
 * @brief アドレス入力バッファ
 *
 * MENU_ADDRESS で使用する。
 */
static char address_input[5];

/**
 * @brief アドレス入力文字数
 */
static uint8_t address_input_index = 0;

/**
 * @brief Bit判定閾値入力バッファ
 *
 * MENU_THRESHOLD で使用する。
 */
static char threshold_input[4];

/**
 * @brief 閾値入力文字数
 */
static uint8_t threshold_input_index;

/**
 * @brief プリアンブル数入力バッファ
 *
 * MENU_PREAMBLE で使用する。
 */
static char preamble_input[3];

/**
 * @brief プリアンブル入力文字数
 */
static uint8_t preamble_input_index;

/**
 * @brief フィルタ名称テーブル
 */
static const char *filter_name[] =
{
    "ALL",
    "Loco",
    "Accessory",
    "OPS"
};

static const char *onoff(bool value);
static const char *address_filter_name(void);

/**
 * @brief 現在の設定内容を表示
 *
 * モニタ画面および設定画面から共通利用される。
 */
static void menu_print_config(void)
{
    printf("--------------------------------\r\n");

    printf(
        " フィルタ    : %s\r\n",
        filter_name[monitor_config.filter]);

    if(monitor_config.address_filter == 0)
    {
        printf(" アドレス    : 無効\r\n");
    }
    else
    {
        printf(
            " アドレス    : %u\r\n",
            monitor_config.address_filter);
    }

    printf(
        " RESET表示   : %s\r\n",
        monitor_config.show_reset ?
            "ON" : "OFF");

    printf(
        " IDLE表示    : %s\r\n",
        monitor_config.show_idle ?
            "ON" : "OFF");

    printf("--------------------------------\r\n");
}

/**
 * @brief メインメニュー表示
 *
 * システムのトップ画面。
 *
 * ユーザーはこの画面から
 *
 * - パケット監視
 * - 統計表示
 * - 設定変更
 *
 * を選択する。
 */
void menu_show_main(void)
{
    current_menu = MENU_MAIN;

    printf("\r\n\r\n");

    printf("======================================\r\n");
    printf(" DCCパケットモニタ Ver1.0.0 (c) Nucky\r\n");
    printf("======================================\r\n");

    printf("\r\n");

    printf(" 1. DCCパケット 表示\r\n");
    printf(" 2. DCCパケット 統計\r\n");
    printf(" 3. DCC 0/1信号 統計\r\n");
    printf(" 4. 設定\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");

    printf("選択 > ");
}

/**
 * @brief パケット監視画面表示
 *
 * DCCパケットのリアルタイム監視を開始する。
 *
 * 監視開始時には
 *
 * - パケット復元状態
 * - パケット通番
 * - 経過時間測定
 *
 * を初期化し、
 * 毎回同じ条件で計測を開始できるようにする。
 */
static void menu_show_monitor(void)
{
    current_menu = MENU_MONITOR;

    /*
     * パケット復元状態初期化
     */
    dcc_packet_init();

    /*
     * 通番・時間計測初期化
     */
    packet_analyzer_init();

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" DCCパケット 表示\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(" 現在設定で表示を開始します\r\n");

    printf("\r\n");

    menu_print_config();

    printf("\r\n");

    printf(" ESC : 戻る\r\n");

    printf("\r\n");
}

/**
 * @brief パケット統計画面表示
 *
 * 受信したDCCパケットの統計情報を表示する。
 *
 * 表示内容
 * - 総パケット数
 * - Packet Rate
 * - RESET割合
 * - IDLE割合
 * - Loco割合
 * - Accessory割合
 * - OPS割合
 * - Service割合
 * - CRCエラー割合
 *
 * コマンドステーションの送出傾向や
 * パケット構成比の調査に利用する。
 */
static void menu_show_packet_stats(void)
{
    uint64_t elapsed_us;
    uint32_t elapsed_sec;
    uint32_t elapsed_ms;

    uint32_t packets_per_sec_x10;

    uint32_t reset_rate_x10;
    uint32_t idle_rate_x10;
    uint32_t loco_rate_x10;
    uint32_t accessory_rate_x10;
    uint32_t ops_rate_x10;
    uint32_t service_rate_x10;
    uint32_t crc_rate_x10;

    current_menu = MENU_PACKET_STATS;

    elapsed_us =
        packet_statistics.elapsed_us;

    elapsed_sec =
        (uint32_t)
        (elapsed_us / 1000000ULL);

    elapsed_ms =
        (uint32_t)
        ((elapsed_us % 1000000ULL)
         / 1000ULL);

    /*
     * Packet Rate計算
     */
    if(elapsed_us == 0)
    {
        packets_per_sec_x10 = 0;
    }
    else
    {
        packets_per_sec_x10 =
            (uint32_t)
            (((uint64_t)
              packet_statistics.total
              * 10000000ULL)
             / elapsed_us);
    }

    /*
     * 各パケット種別の割合計算
     */
    if(packet_statistics.total == 0)
    {
        reset_rate_x10     = 0;
        idle_rate_x10      = 0;
        loco_rate_x10      = 0;
        accessory_rate_x10 = 0;
        ops_rate_x10       = 0;
        service_rate_x10   = 0;
        crc_rate_x10       = 0;
    }
    else
    {
        reset_rate_x10 =
            (packet_statistics.reset * 1000U)
            / packet_statistics.total;

        idle_rate_x10 =
            (packet_statistics.idle * 1000U)
            / packet_statistics.total;

        loco_rate_x10 =
            (packet_statistics.loco * 1000U)
            / packet_statistics.total;

        accessory_rate_x10 =
            (packet_statistics.accessory * 1000U)
            / packet_statistics.total;

        ops_rate_x10 =
            (packet_statistics.ops * 1000U)
            / packet_statistics.total;

        service_rate_x10 =
            (packet_statistics.service * 1000U)
            / packet_statistics.total;

        crc_rate_x10 =
            (packet_statistics.crc_error * 1000U)
            / packet_statistics.total;
    }

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" DCCパケット 統計\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(
        " 測定時間         : %lu.%03lus\r\n",
        (unsigned long)elapsed_sec,
        (unsigned long)elapsed_ms);

    printf(
        " 平均 Packet Rate : %lu.%01lu pkt/s\r\n",
        (unsigned long)
            (packets_per_sec_x10 / 10),
        (unsigned long)
            (packets_per_sec_x10 % 10));

    printf("\r\n");

    printf(
        " Total       : %lu\r\n",
        (unsigned long)
        packet_statistics.total);

    printf("\r\n");

    printf(
        " Reset       : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.reset,
        (unsigned long)
            (reset_rate_x10 / 10),
        (unsigned long)
            (reset_rate_x10 % 10));

    printf(
        " Idle        : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.idle,
        (unsigned long)
            (idle_rate_x10 / 10),
        (unsigned long)
            (idle_rate_x10 % 10));

    printf("\r\n");

    printf(
        " Loco        : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.loco,
        (unsigned long)
            (loco_rate_x10 / 10),
        (unsigned long)
            (loco_rate_x10 % 10));

    printf(
        " Accessory   : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.accessory,
        (unsigned long)
            (accessory_rate_x10 / 10),
        (unsigned long)
            (accessory_rate_x10 % 10));

    printf("\r\n");

    printf(
        " OPS         : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.ops,
        (unsigned long)
            (ops_rate_x10 / 10),
        (unsigned long)
            (ops_rate_x10 % 10));

    printf(
        " Service     : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.service,
        (unsigned long)
            (service_rate_x10 / 10),
        (unsigned long)
            (service_rate_x10 % 10));

    printf("\r\n");

    printf(
        " CRC Error   : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            packet_statistics.crc_error,
        (unsigned long)
            (crc_rate_x10 / 10),
        (unsigned long)
            (crc_rate_x10 % 10));

    printf("\r\n");

    printf(" R   : 統計クリア\r\n");
    printf(" ESC : 戻る\r\n");
}

/**
 * @brief DCC信号統計画面表示
 *
 * DCC信号そのものの品質を表示する。
 *
 * 表示内容
 * - 測定時間
 * - Bit Rate
 * - Bit0 / Bit1 出現率
 * - Bit0 Mode
 * - Bit1 Mode
 * - パルス幅分布
 *
 * 本画面はパケット解析ではなく、
 * DCC波形品質の確認を目的としている。
 *
 * コマンドステーション比較や
 * Bit判定閾値調整時に利用する。
 */
static void menu_show_signal_stats(void)
{
    uint64_t elapsed_us;

    uint32_t elapsed_sec;
    uint32_t elapsed_ms;

    uint32_t total_bit;

    uint32_t bit_rate_x10;

    uint32_t bit0_rate_x10;
    uint32_t bit1_rate_x10;

    uint32_t bit1_mode_us;
    uint32_t bit0_mode_us;

    uint32_t short_pulse_count;

    uint32_t bit1_range_count;

    uint32_t bit0_range_count;

    current_menu = MENU_SIGNAL_STATS;

    elapsed_us =
        packet_statistics.elapsed_us;

    elapsed_sec =
        (uint32_t)
        (elapsed_us / 1000000ULL);

    elapsed_ms =
        (uint32_t)
        ((elapsed_us % 1000000ULL)
         / 1000ULL);

    total_bit =
        signal_statistics.bit0_count +
        signal_statistics.bit1_count;

    /*
     * 平均Bit Rate計算
     */
    if(elapsed_us == 0)
    {
        bit_rate_x10 = 0;
    }
    else
    {
        bit_rate_x10 =
            (uint32_t)
            (((uint64_t)
              total_bit
              * 10000000ULL)
             / elapsed_us);
    }

    /*
     * Bit0 / Bit1 出現率計算
     */
    if(total_bit == 0)
    {
        bit0_rate_x10 = 0;
        bit1_rate_x10 = 0;
    }
    else
    {
        bit0_rate_x10 =
            (signal_statistics.bit0_count
             * 1000U)
            / total_bit;

        bit1_rate_x10 =
            (signal_statistics.bit1_count
             * 1000U)
            / total_bit;
    }

    /*
     * 最頻出パルス幅取得
     */
    bit1_mode_us =
        statistics_get_bit1_mode();

    bit0_mode_us =
        statistics_get_bit0_mode();

    /*
     * パルス幅分布集計
     */
    short_pulse_count =
        statistics_get_range_count(
            HISTOGRAM_MIN_US,
            SHORT_PULSE_MAX_US);

    bit1_range_count =
        statistics_get_range_count(
            BIT1_RANGE_MIN_US,
            monitor_config.threshold_us);

    bit0_range_count =
        statistics_get_range_count(
            monitor_config.threshold_us + 1,
            HISTOGRAM_MAX_US);

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" DCC 0/1信号 統計\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(
        " 測定時間      : %lu.%03lus\r\n",
        (unsigned long)elapsed_sec,
        (unsigned long)elapsed_ms);

    printf(
        " 平均 Bit Rate : %lu.%01lu bit/s\r\n",
        (unsigned long)
            (bit_rate_x10 / 10),
        (unsigned long)
            (bit_rate_x10 % 10));

    printf("\r\n");

    printf(
        " Total Bit   : %5lu\r\n",
        (unsigned long)total_bit);

    printf("\r\n");

    printf(
        " Bit1        : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            signal_statistics.bit1_count,
        (unsigned long)
            (bit1_rate_x10 / 10),
        (unsigned long)
            (bit1_rate_x10 % 10));

    printf(
        " Bit0        : %5lu (%2lu.%01lu%%)\r\n",
        (unsigned long)
            signal_statistics.bit0_count,
        (unsigned long)
            (bit0_rate_x10 / 10),
        (unsigned long)
            (bit0_rate_x10 % 10));

    printf("\r\n");

    printf(
        " Bit1 Mode   : %5luus\r\n",
        (unsigned long)
            bit1_mode_us);

    printf(
        " Bit0 Mode   : %5luus\r\n",
        (unsigned long)
            bit0_mode_us);

    printf("\r\n");

    printf(
        " Short Pulse 0-54us      : %lu\r\n",
        (unsigned long)
            short_pulse_count);

    printf(
        " Bit1 Range  55-%uus     : %lu\r\n",
        monitor_config.threshold_us,
        (unsigned long)
            bit1_range_count);

    printf(
        " Bit0 Range  %u-300us    : %lu\r\n",
        monitor_config.threshold_us + 1,
        (unsigned long)
            bit0_range_count);

    printf(
        " Long Pulse  301-10000us : %lu\r\n",
        (unsigned long)
            signal_statistics.
                range_301_10000us_count);

    printf(
        " Signal Loss Over10000us : %lu\r\n",
        (unsigned long)
            signal_statistics.
                over_10000us_count);

    printf("\r\n");

    printf(" H   : ヒストグラム表示\r\n");
    printf(" R   : 統計クリア\r\n");
    printf(" ESC : 戻る\r\n");
}

/**
 * @brief DCC信号ヒストグラム表示
 *
 * 0～300us のパルス幅分布を表示する。
 *
 * ヒストグラムは1us単位で集計されており、
 * DCC波形品質の評価やノイズ解析に利用できる。
 *
 * この画面は signal_stats 画面の詳細表示版である。
 */
static void menu_show_signal_histogram(void)
{
    uint16_t i;

    uint32_t bit1_mode_us;
    uint32_t bit0_mode_us;

    uint32_t short_pulse_count;

    uint32_t bit1_range_count;

    uint32_t bit0_range_count;

    current_menu =
        MENU_SIGNAL_HISTOGRAM;

    bit1_mode_us =
        statistics_get_bit1_mode();

    bit0_mode_us =
        statistics_get_bit0_mode();

    short_pulse_count =
        statistics_get_range_count(
            0,
            54);

    bit1_range_count =
        statistics_get_range_count(
            55,
            monitor_config.threshold_us);

    bit0_range_count =
        statistics_get_range_count(
            monitor_config.threshold_us + 1,
            300);

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" ヒストグラム表示（0-300us）\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(
        " Bit1 Mode   : %5luus\r\n",
        (unsigned long)
            bit1_mode_us);

    printf(
        " Bit0 Mode   : %5luus\r\n",
        (unsigned long)
            bit0_mode_us);

    printf("\r\n");

    printf(
        " Short Pulse 0-54us      : %lu\r\n",
        (unsigned long)
            short_pulse_count);

    printf(
        " Bit1 Range  55-%uus     : %lu\r\n",
        monitor_config.threshold_us,
        (unsigned long)
            bit1_range_count);

    printf(
        " Bit0 Range  %u-300us    : %lu\r\n",
        monitor_config.threshold_us + 1,
        (unsigned long)
            bit0_range_count);

    printf(
        " Long Pulse  301-10000us : %lu\r\n",
        (unsigned long)
            signal_statistics.
                range_301_10000us_count);

    printf(
        " Signal Loss Over10000us : %lu\r\n",
        (unsigned long)
            signal_statistics.
                over_10000us_count);

    printf("\r\n");

    printf(
        "-----------------------------\r\n");

    printf("\r\n");

    /*
     * 発生回数が0以外の項目のみ表示
     */
    for(i = 0;
        i < HISTOGRAM_BUCKET_COUNT;
        i++)
    {
        if(signal_statistics.hist_0_300[i] == 0)
        {
            continue;
        }

        printf(
            "%3u us : %lu\r\n",
            (unsigned int)i,
            (unsigned long)
                signal_statistics.hist_0_300[i]);
    }

    printf("\r\n");

    printf(" ESC : 戻る\r\n");
}

/**
 * @brief 設定メニュー表示
 *
 * モニタ動作に関する設定画面を表示する。
 *
 * ユーザーは本画面から以下を変更できる。
 *
 * - パケット種別フィルタ
 * - アドレスフィルタ
 * - RESETパケット表示
 * - IDLEパケット表示
 * - Bit判定閾値
 * - プリアンブル判定数
 *
 * 実際の設定値は monitor_config が保持する。
 */
static void menu_show_settings(void)
{
    current_menu = MENU_SETTINGS;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" 設定\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(" 1. パケット種別フィルタ  [%s]\r\n",
           filter_name[monitor_config.filter]);

    printf(" 2. アドレスフィルタ     [%s]\r\n",
           address_filter_name());

    printf(" 3. RESET表示 ON/OFF    [%s]\r\n",
           onoff(monitor_config.show_reset));

    printf(" 4. IDLE表示 ON/OFF     [%s]\r\n",
           onoff(monitor_config.show_idle));

    printf(" 5. DCC 0/1判定 閾値    [%uus]\r\n",
           monitor_config.threshold_us);

    printf(" 6. プリアンブル数      [%u]\r\n",
           monitor_config.preamble_count);

    printf("\r\n");

    printf(" ESC : 戻る\r\n");

    printf("\r\n");

    printf("選択 > ");
}

/**
 * @brief パケット種別フィルタ設定画面
 *
 * 表示するパケット種別を選択する。
 *
 * FILTER_ALL
 *   全パケット表示
 *
 * FILTER_LOCO
 *   機関車パケットのみ
 *
 * FILTER_ACCESSORY
 *   アクセサリパケットのみ
 *
 * FILTER_OPS
 *   OPSプログラミングのみ
 */
static void menu_show_filter(void)
{
    current_menu = MENU_FILTER;

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" パケット種別フィルタ\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(" 現在 : %s\r\n",
           filter_name[monitor_config.filter]);

    printf("\r\n");

    printf(" 1. ALL\r\n");
    printf(" 2. Loco\r\n");
    printf(" 3. Accessory\r\n");
    printf(" 4. OPS\r\n");

    printf("\r\n");

    printf(" ESC : 戻る\r\n");
    printf("\r\n");

    printf("選択 > ");
}

/**
 * @brief アドレスフィルタ設定画面
 *
 * 指定したアドレスのパケットのみ表示する。
 *
 * 用途例
 * - 特定車両の調査
 * - 特定アクセサリの調査
 * - 開発中デコーダの監視
 *
 * 0を指定するとフィルタ無効となる。
 */
static void menu_show_address(void)
{
    current_menu = MENU_ADDRESS;

    address_input_index = 0;
    address_input[0] = '\0';

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" アドレスフィルタ\r\n");
    printf("================================\r\n");

    printf("\r\n");

    if(monitor_config.address_filter == 0)
    {
        printf(" 現在 : 無効\r\n");
    }
    else
    {
        printf(" 現在 : %u\r\n",
               monitor_config.address_filter);
    }

    printf("\r\n");

    printf(" 0      : フィルタ無効\r\n");
    printf(" 1-9999 : アドレス指定\r\n");

    printf("\r\n");

    printf(" ENTER : 変更なし\r\n");
    printf(" ESC   : 戻る\r\n");

    printf("\r\n");

    printf(" 入力 > ");
}

/**
 * @brief Bit判定閾値設定画面
 *
 * dcc_decode が使用する
 * Bit0 / Bit1 判定境界値を設定する。
 *
 * 通常のDCC信号では
 * 70～90us付近が目安となる。
 *
 * コマンドステーション評価や
 * 誤判定調査に利用する。
 */
static void menu_show_threshold(void)
{
    current_menu =
        MENU_THRESHOLD;

    threshold_input_index = 0;

    threshold_input[0] = '\0';

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" DCC 0/1判定閾値 設定\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(
        " 現在 : %uus\r\n",
        monitor_config.threshold_us);

    printf("\r\n");

    printf(
        " 20-200 : 閾値入力\r\n");

    printf("\r\n");

    printf(" ENTER : 変更なし\r\n");
    printf(" ESC   : 戻る\r\n");

    printf("\r\n");

    printf("入力 > ");
}

/**
 * @brief プリアンブル数設定画面
 *
 * dcc_packet がパケット開始と判定するために必要な
 * 連続Bit1数を設定する。
 *
 * NMRA規格では10bit以上のプリアンブルが定義される。
 *
 * 信号品質評価や特殊な信号解析用途のため
 * ユーザー設定可能としている。
 */
static void menu_show_preamble(void)
{
    current_menu =
        MENU_PREAMBLE;

    preamble_input_index = 0;

    preamble_input[0] = '\0';

    printf("\r\n\r\n");

    printf("================================\r\n");
    printf(" プリアンブル数 設定\r\n");
    printf("================================\r\n");

    printf("\r\n");

    printf(
        " 現在 : %u\r\n",
        monitor_config.preamble_count);

    printf("\r\n");

    printf(
        " 10-30 : プリアンブル数\r\n");

    printf("\r\n");

    printf(" ENTER : 変更なし\r\n");
    printf(" ESC   : 戻る\r\n");

    printf("\r\n");

    printf("入力 > ");
}

/**
 * @brief キー入力処理
 *
 * 現在のメニュー状態に応じて
 * 入力文字を解釈する。
 *
 * 本関数はUIステートマシンの中心であり、
 * 画面遷移および設定変更を管理する。
 *
 * ESCキーは共通キャンセル操作として扱い、
 * 一つ上の階層へ戻る。
 *
 * @param ch 入力文字
 */
void menu_process_key(int ch)
{
    /*
     * ESCキー処理
     *
     * 現在の画面状態に応じて
     * 一つ前のメニューへ戻る。
     */
    if(ch == 27)
    {
        switch(current_menu)
        {
            case MENU_MAIN:
                break;

            case MENU_SETTINGS:
                menu_show_main();
                break;

            case MENU_FILTER:
                menu_show_settings();
                break;

            case MENU_ADDRESS:

                address_input_index = 0;
                address_input[0] = '\0';

                menu_show_settings();
                break;

            case MENU_THRESHOLD:

                threshold_input_index = 0;
                threshold_input[0] = '\0';

                menu_show_settings();
                break;

            case MENU_PREAMBLE:

                preamble_input_index = 0;
                preamble_input[0] = '\0';

                menu_show_settings();
                break;

            case MENU_MONITOR:

                /*
                 * パケット監視終了
                 */
                menu_show_main();
                break;

            case MENU_PACKET_STATS:

                /*
                 * パケット統計終了
                 */
                menu_show_main();
                break;

            case MENU_SIGNAL_STATS:

                /*
                 * 信号統計終了
                 */
                menu_show_main();
                break;

            case MENU_SIGNAL_HISTOGRAM:

                /*
                 * ヒストグラム画面から
                 * 信号統計画面へ戻る
                 */
                menu_show_signal_stats();
                break;

            default:

                menu_show_main();
                break;
        }

        return;
    }

    /*
     * 現在の画面状態ごとに
     * 入力を解釈する。
     */
    switch(current_menu)
    {
        case MENU_NONE:

            /*
             * 起動画面。
             *
             * ENTERで
             * メインメニューへ遷移。
             */
            if((ch == '\r') ||
            (ch == '\n'))
            {
                menu_show_main();
            }

            break;

        case MENU_MAIN:

            /*
             * メインメニュー選択
             */
            switch(ch)
            {
                case '1':

                    menu_show_monitor();
                    break;

                case '2':

                    menu_show_packet_stats();
                    break;

                case '3':

                    menu_show_signal_stats();
                    break;

                case '4':

                    menu_show_settings();
                    break;

                default:
                    break;
            }

            break;

        case MENU_MONITOR:

            /*
             * 監視画面では
             * ESCのみ受け付ける。
             */
            break;

        case MENU_PACKET_STATS:

            /*
             * Rキーで統計リセット。
             */
            if((ch == 'r') ||
            (ch == 'R'))
            {
                statistics_reset();

                menu_show_packet_stats();
            }

            break;

        case MENU_SIGNAL_STATS:

            /*
             * Hキーで
             * ヒストグラム表示。
             */
            if((ch == 'h') ||
            (ch == 'H'))
            {
                menu_show_signal_histogram();
                return;
            }

            /*
             * Rキーで統計リセット。
             */
            if((ch == 'r') ||
            (ch == 'R'))
            {
                statistics_reset();
                menu_show_signal_stats();
            }

            break;

        case MENU_SETTINGS:

            /*
             * 設定メニュー。
             *
             * 各設定画面への入口となる。
             */
            switch(ch)
            {
                case '1':

                    menu_show_filter();
                    break;

                case '2':

                    menu_show_address();
                    break;

                case '3':

                    /*
                     * RESETパケット表示切替
                     */
                    monitor_config.show_reset =
                        !monitor_config.show_reset;

                    menu_show_settings();

                    break;

                case '4':

                    /*
                     * IDLEパケット表示切替
                     */
                    monitor_config.show_idle =
                        !monitor_config.show_idle;

                    menu_show_settings();

                    break;

                case '5':

                    menu_show_threshold();
                    break;

                case '6':

                    menu_show_preamble();
                    break;

                default:
                    break;
            }

            break;

        case MENU_FILTER:

            /*
             * パケット種別フィルタ設定。
             *
             * packet_analyzer が参照する
             * monitor_config.filter を変更する。
             */
            switch(ch)
            {
                case '1':
                    monitor_config.filter =
                        FILTER_ALL;
                    break;

                case '2':
                    monitor_config.filter =
                        FILTER_LOCO;
                    break;

                case '3':
                    monitor_config.filter =
                        FILTER_ACCESSORY;
                    break;

                case '4':
                    monitor_config.filter =
                        FILTER_OPS;
                    break;

                default:
                    return;
            }

            /*
             * 設定後は設定画面へ戻る。
             */
            menu_show_settings();

            break;

        case MENU_ADDRESS:

            /*
             * アドレスフィルタ入力。
             *
             * 0      : フィルタ無効
             * 1-9999 : 指定アドレスのみ表示
             */
            if(ch == '\r' || ch == '\n')
            {
                /*
                 * 何も入力されていない場合は
                 * 変更なしとして戻る。
                 */
                if(address_input_index == 0)
                {
                    menu_show_settings();
                    return;
                }

                uint16_t value;

                value =
                    (uint16_t)
                    atoi(address_input);

                /*
                 * 有効範囲チェック
                 */
                if(value <= 9999)
                {
                    monitor_config.address_filter =
                        value;
                }

                address_input_index = 0;
                address_input[0] = '\0';

                menu_show_settings();

                return;
            }

            /*
             * Backspace処理
             */
            if((ch == 8) || (ch == 127))
            {
                if(address_input_index > 0)
                {
                    address_input_index--;

                    address_input[address_input_index] =
                        '\0';

                    printf("\b \b");
                }

                break;
            }

            /*
             * 数字のみ受け付ける
             */
            if(ch >= '0' && ch <= '9')
            {
                if(address_input_index < 4)
                {
                    address_input[address_input_index++] =
                        (char)ch;

                    address_input[address_input_index] =
                        '\0';

                    putchar(ch);
                }
            }

            break;

        case MENU_THRESHOLD:

            /*
             * Bit判定閾値入力。
             *
             * dcc_decode が参照する
             * Bit0 / Bit1 判定境界値を変更する。
             */
            if(ch == '\r' || ch == '\n')
            {
                /*
                 * 入力なしなら変更せず戻る。
                 */
                if(threshold_input_index == 0)
                {
                    menu_show_settings();

                    return;
                }

                uint16_t value;

                value =
                    (uint16_t)
                    atoi(threshold_input);

                /*
                 * 実用範囲のみ許可。
                 */
                if((value >= 20) &&
                   (value <= 200))
                {
                    monitor_config.threshold_us =
                        value;
                }

                threshold_input_index = 0;

                threshold_input[0] =
                    '\0';

                menu_show_settings();

                return;
            }

            /*
             * Backspace処理
             */
            if((ch == 8) ||
               (ch == 127))
            {
                if(threshold_input_index > 0)
                {
                    threshold_input_index--;

                    threshold_input[
                        threshold_input_index] =
                            '\0';

                    printf("\b \b");
                }

                break;
            }

            /*
             * 数字のみ受け付ける
             */
            if(ch >= '0' &&
               ch <= '9')
            {
                if(threshold_input_index < 3)
                {
                    threshold_input[
                        threshold_input_index++] =
                            (char)ch;

                    threshold_input[
                        threshold_input_index] =
                            '\0';

                    putchar(ch);
                }
            }

            break;

        case MENU_PREAMBLE:

            /*
             * プリアンブル数入力。
             *
             * dcc_packet が参照する
             * パケット開始判定条件を変更する。
             */
            if(ch == '\r' || ch == '\n')
            {
                /*
                 * 入力なしなら変更せず戻る。
                 */
                if(preamble_input_index == 0)
                {
                    menu_show_settings();

                    return;
                }

                uint16_t value;

                value =
                    (uint16_t)
                    atoi(preamble_input);

                /*
                 * NMRAおよび実運用を考慮した範囲。
                 */
                if((value >= 10) &&
                   (value <= 30))
                {
                    monitor_config.preamble_count =
                        value;
                }

                preamble_input_index = 0;

                preamble_input[0] =
                    '\0';

                menu_show_settings();

                return;
            }

            /*
             * Backspace処理
             */
            if((ch == 8) ||
               (ch == 127))
            {
                if(preamble_input_index > 0)
                {
                    preamble_input_index--;

                    preamble_input[
                        preamble_input_index] =
                            '\0';

                    printf("\b \b");
                }

                break;
            }

            /*
             * 数字のみ受け付ける
             */
            if(ch >= '0' &&
               ch <= '9')
            {
                if(preamble_input_index < 2)
                {
                    preamble_input[
                        preamble_input_index++] =
                            (char)ch;

                    preamble_input[
                        preamble_input_index] =
                            '\0';

                    putchar(ch);
                }
            }

            break;

        default:

            /*
             * 未使用状態
             */
            break;
    }
}

/**
 * @brief ON/OFF文字列取得
 *
 * 真偽値を表示用文字列へ変換する。
 *
 * 設定画面で
 *
 * - RESET表示
 * - IDLE表示
 *
 * の状態表示に使用する。
 *
 * @param value 真偽値
 *
 * @retval "ON"  true
 * @retval "OFF" false
 */
static const char *onoff(bool value)
{
    return value ? "ON" : "OFF";
}

/**
 * @brief アドレスフィルタ表示文字列取得
 *
 * 設定画面で使用する表示用ヘルパ関数。
 *
 * address_filter が 0 の場合は
 * フィルタ無効として扱う。
 *
 * @return 表示用文字列
 */
static const char *address_filter_name(void)
{
    static char text[16];

    /*
     * address_filter = 0
     * はフィルタ無効を意味する。
     */
    if(monitor_config.address_filter == 0)
    {
        return "無効";
    }

    snprintf(
        text,
        sizeof(text),
        "%u",
        monitor_config.address_filter);

    return text;
}

/**
 * @brief パケット監視画面状態取得
 *
 * packet_analyzer() が
 * パケット表示を行うべきか判定するために使用する。
 *
 * DCC受信自体は常時継続されるが、
 * 監視画面以外では表示処理を抑止する。
 *
 * @retval true  MENU_MONITOR
 * @retval false その他画面
 */
bool menu_is_monitor_mode(void)
{
    return (current_menu == MENU_MONITOR);
}
