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
 * @file packet_analyzer.c
 * @brief DCCパケット解析および表示処理
 *
 * dcc_packet モジュールが復元した DCC_PACKET を解析し、
 * モニタ表示および統計情報更新を行う。
 *
 * 本モジュールは DCC Packet Monitor の中心となる
 * パケット解析レイヤであり、以下の機能を担当する。
 *
 * - パケット通番管理
 * - パケット間隔計測
 * - CRC検査
 * - パケット種別判定
 * - フィルタ処理
 * - 統計カウンタ更新
 * - テキスト表示
 *
 * パケット内容の詳細解析は、
 * 各 analyzer モジュールへ委譲する。
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "packet_analyzer.h"
#include "loco_analyzer.h"
#include "accessory_analyzer.h"
#include "service_analyzer.h"
#include "ops_analyzer.h"
#include "monitor_config.h"
#include "statistics.h"

/**
 * @brief 表示時の最大HEXバイト数
 *
 * 表示列幅固定のため使用する。
 *
 * 長さが不足する場合は空白で埋める。
 */
#define HEX_DISPLAY_WIDTH 6

/**
 * @brief 表示用パケット通番
 *
 * 監視開始ごとに 1 から開始する。
 *
 * ユーザーがパケットを追跡しやすくするために
 * 表示専用の通番を採用している。
 */
static uint32_t packet_no;

/**
 * @brief 前回パケット受信時刻
 *
 * パケット間隔表示の基準となる。
 *
 * timestamp_us は dcc_packet が付与した
 * 受信時刻情報を使用する。
 */
static uint32_t last_timestamp;

/**
 * @brief パケット解析状態初期化
 *
 * 監視開始時に呼び出される。
 *
 * 以下を初期化する。
 *
 * - パケット通番
 * - パケット間隔計測基準
 *
 * これにより毎回同一条件で
 * 測定を開始できる。
 */
void packet_analyzer_init(void)
{
    packet_no = 1;
    last_timestamp = 0;
}

/**
 * @brief DCCパケットCRC検査
 *
 * DCCパケットでは
 * 最終バイトが XOR CRC として格納される。
 *
 * 本関数は受信データのCRC整合性を検証する。
 *
 * @param packet 対象パケット
 *
 * @retval true  CRC正常
 * @retval false CRC異常
 */
static bool check_crc(
    const DCC_PACKET *packet)
{
    uint8_t crc = 0;

    /*
     * CRCバイトを含めて
     * 最低2バイト必要。
     */
    if(packet->length < 2)
    {
        return false;
    }

    /*
     * CRCバイトを除く全データを
     * XOR計算する。
     */
    for(uint8_t i = 0;
        i < (packet->length - 1);
        i++)
    {
        crc ^= packet->data[i];
    }

    /*
     * 計算結果と受信CRCを比較する。
     */
    return
        (crc ==
         packet->data[
             packet->length - 1]);
}

/**
 * @brief 機関車速度制御パケット判定
 *
 * loco_analyzer が
 * [128SPD] と表示するパケットを検出する。
 *
 * @param packet 対象パケット
 *
 * @retval true  Speedパケット
 * @retval false それ以外
 */
static bool is_loco_speed_packet(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    if(packet == NULL)
    {
        return false;
    }

    if((packet->data[0] & 0xC0) == 0xC0)
    {
        if(packet->length < 4)
        {
            return false;
        }

        inst = packet->data[2];
    }
    else
    {
        if(packet->length < 3)
        {
            return false;
        }

        inst = packet->data[1];
    }

    return (inst == 0x3F);
}

/**
 * @brief 機関車ファンクションパケット判定
 *
 * F0～F68制御命令を検出する。
 *
 * @param packet 対象パケット
 *
 * @retval true  Functionパケット
 * @retval false それ以外
 */
static bool is_loco_function_packet(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    if(packet == NULL)
    {
        return false;
    }

    if((packet->data[0] & 0xC0) == 0xC0)
    {
        if(packet->length < 4)
        {
            return false;
        }

        inst = packet->data[2];
    }
    else
    {
        if(packet->length < 3)
        {
            return false;
        }

        inst = packet->data[1];
    }

    if((inst & 0xE0) == 0x80)
    {
        return true;
    }

    if((inst & 0xF0) == 0xB0)
    {
        return true;
    }

    if((inst & 0xF0) == 0xA0)
    {
        return true;
    }

    if(inst == 0xDE ||
       inst == 0xDF ||
      (inst >= 0xD8 &&
       inst <= 0xDC))
    {
        return true;
    }

    return false;
}



/**
 * @brief パケットからアドレスを取得
 *
 * アドレスフィルタ処理のため、
 * DCCパケットから対象アドレスを抽出する。
 *
 * 対応するアドレス形式
 *
 * - Short Address
 * - Long Address
 * - Accessory Address
 *
 * DCCではパケット種別によって
 * アドレス格納形式が異なるため、
 * 本関数で吸収する。
 *
 * @param packet 対象パケット
 *
 * @return フィルタ比較用アドレス
 */
static uint16_t get_packet_address(
    const DCC_PACKET *packet)
{
    /*
     * 念のためNULLチェック。
     */
    if(packet == NULL)
    {
        return 0;
    }

    /*
     * Accessory Packet
     *
     * NMRA S-9.2.1 の
     * Basic Accessory Decoder Address
     * をデコーダアドレスへ変換する。
     */
    if((packet->data[0] & 0xC0) == 0x80)
    {
        uint16_t addr_9bit;
        uint8_t pair;

        /*
         * Accessory Address は
         * 9bitアドレスとして符号化される。
         *
         * A8,A7,A6 は反転状態で
         * 2バイト目へ格納されているため、
         * ビット反転して復元する。
         */
        addr_9bit =
            ((uint16_t)
             ((~packet->data[1]) & 0x70)
             << 2)
            |
            (packet->data[0] & 0x3F);

        /*
         * 出力番号(1～4)
         *
         * 1つのアクセサリアドレスは
         * 4出力を保持する。
         */
        pair =
            (packet->data[1] >> 1) & 0x03;

        /*
         * NMRA表現から
         * ユーザーが一般的に使用する
         * アクセサリアドレスへ変換する。
         *
         * 例:
         * Address 1～4
         * Address 5～8
         * Address 9～12
         */
        return
            (addr_9bit > 0)
            ? ((addr_9bit - 1) * 4)
                + pair + 1
            : pair + 1;
    }

    /*
     * Long Address
     *
     * 先頭2bit = 11
     *
     * 14bitアドレスとして
     * 上位6bit＋下位8bitを結合する。
     */
    if((packet->data[0] & 0xC0) == 0xC0)
    {
        return
            ((uint16_t)
             (packet->data[0] & 0x3F)
             << 8)
             |
             packet->data[1];
    }

    /*
     * Short Address
     *
     * 1バイト目そのものが
     * アドレス値となる。
     */
    return packet->data[0];
}

/**
 * @brief DCCパケット解析
 *
 * 受信したDCCパケットを解析し、
 * 表示および統計更新を行う。
 *
 * 処理の流れ
 *
 * 1. パケット妥当性確認
 * 2. フィルタ判定
 * 3. 経過時間計算
 * 4. 通番更新
 * 5. CRC確認
 * 6. 種別分類
 * 7. 詳細解析
 *
 * @param packet 解析対象パケット
 */
void packet_analyzer(
    const DCC_PACKET *packet)
{
    uint32_t delta_us;
    uint32_t delta_ms;
    uint32_t delta_frac;
    uint32_t display_no;

    /*
     * フィルタ判定結果。
     *
     * true  : 表示する
     * false : 表示しない
     */
    bool show_packet = true;

    /*
     * NULLパケット防止
     */
    if(packet == NULL)
    {
        return;
    }

    /*
     * 総受信パケット数を加算する。
     *
     * 表示有無に関係なく、
     * 実際に受信した全パケットを統計対象とする。
     */
    statistics_count_total();

    /*
     * RESETパケット表示制御
     *
     * RESETは頻繁に送信されるため、
     * ユーザー設定により非表示化できる。
     */
    if(packet->length == 3 &&
       packet->data[0] == 0x00 &&
       packet->data[1] == 0x00 &&
       packet->data[2] == 0x00)
    {
        if(!monitor_config.show_reset)
        {
            show_packet = false;
        }
    }

    /*
     * IDLEパケット表示制御
     *
     * IDLEもコマンドステーションから
     * 大量に送出されるため、
     * 必要に応じて非表示にできる。
     */
    if(packet->length == 3 &&
       packet->data[0] == 0xFF &&
       packet->data[1] == 0x00 &&
       packet->data[2] == 0xFF)
    {
        if(!monitor_config.show_idle)
        {
            show_packet = false;
        }
    }

    /*
     * パケット種別フィルタ
     *
     * ユーザーが指定した種別のみを
     * 表示対象とする。
     *
     * 注意:
     * 通番および時間計測は
     * フィルタ前の全パケット基準で継続する。
     */
    switch(monitor_config.filter)
    {
        case FILTER_ALL:

            /*
             * 全表示
             */
            break;

        case FILTER_LOCO:

            /*
             * 機関車制御関連のみ表示
             *
             * 以下を除外する。
             * - RESET
             * - IDLE
             * - Service Mode
             * - OPS Mode
             * - Accessory
             */
            if(packet->data[0] == 0x00)
            {
                show_packet = false;
            }
            else if(packet->length == 3 &&
                    packet->data[0] == 0xFF &&
                    packet->data[1] == 0x00 &&
                    packet->data[2] == 0xFF)
            {
                show_packet = false;
            }
            else if(is_service_packet(packet))
            {
                show_packet = false;
            }
            else if(is_ops_packet(packet))
            {
                show_packet = false;
            }
            else if((packet->data[0] & 0xC0) == 0x80)
            {
                show_packet = false;
            }

            break;

        case FILTER_LOCO_SPEED:

            if(!is_loco_speed_packet(packet))
            {
                show_packet = false;
            }

            break;

        case FILTER_LOCO_FUNCTION:

            if(!is_loco_function_packet(packet))
            {
                show_packet = false;
            }

            break;

        case FILTER_ACCESSORY:

            /*
             * アクセサリ制御のみ表示
             *
             * ポイントや信号機などの
             * デコーダ解析用。
             */
            if(packet->data[0] == 0x00)
            {
                show_packet = false;
            }
            else if(packet->length == 3 &&
                    packet->data[0] == 0xFF &&
                    packet->data[1] == 0x00 &&
                    packet->data[2] == 0xFF)
            {
                show_packet = false;
            }
            else if(is_service_packet(packet))
            {
                show_packet = false;
            }
            else if(is_ops_packet(packet))
            {
                show_packet = false;
            }
            else if((packet->data[0] & 0xC0) != 0x80)
            {
                show_packet = false;
            }

            break;

        case FILTER_OPS:

            /*
             * OPS Mode Packetのみ表示
             *
             * 本線上CV変更解析用途。
             */
            if(!is_ops_packet(packet))
            {
                show_packet = false;
            }

            break;

        default:
            break;
    }

    /*
     * アドレスフィルタ
     *
     * monitor_config.address_filter
     * が 0 以外の場合のみ有効。
     */
    if(monitor_config.address_filter != 0)
    {
        uint16_t address;

        address =
            get_packet_address(packet);

        if(address !=
           monitor_config.address_filter)
        {
            show_packet = false;
        }
    }

    /*
     * 前回パケットからの経過時間計算
     *
     * CRCエラーやフィルタ除外パケットも
     * 時間基準の対象とする。
     *
     * これにより表示される時間軸が
     * 常に実際の受信順序と一致する。
     */
    if(last_timestamp == 0)
    {
        delta_us = 0;
    }
    else
    {
        delta_us =
            packet->timestamp_us
            - last_timestamp;
    }

    /*
     * 次回計算用に保存
     */
    last_timestamp =
        packet->timestamp_us;

    /*
     * 統計用経過時間加算
     */
    statistics_add_elapsed_us(
        delta_us);

    /*
     * 通番管理
     *
     * フィルタ除外やCRCエラーも含め、
     * 全パケット基準で採番する。
     *
     * この方式の方が測定器として
     * 一貫性が高い。
     */
    display_no = packet_no;

    packet_no++;

    if(packet_no > 999999)
    {
        packet_no = 1;
    }

    /*
     * 表示対象外なら終了。
     *
     * 通番と時間計測は
     * 既に更新済みである。
     */
    if(!show_packet)
    {
        return;
    }

    /*
     * 表示用時間変換
     *
     * us単位で計測しているため、
     * 表示時のみ ms.xxx 形式へ変換する。
     */
    delta_ms   = delta_us / 1000;
    delta_frac = delta_us % 1000;

    /*
     * パケットヘッダ表示
     *
     * 表示フォーマット
     *
     * 000001  001.234ms  FF 00 FF ...
     */
    printf("%06lu  %03lu.%03lums  ",
        (unsigned long)display_no,
        (unsigned long)delta_ms,
        (unsigned long)delta_frac);

    /*
     * パケット内容をHEX表示する。
     *
     * 表示幅固定にすることで
     * 後続テキストの位置ずれを防止する。
     */
    for(uint8_t i = 0;
        i < HEX_DISPLAY_WIDTH;
        i++)
    {
        if(i < packet->length)
        {
            printf("%02X ",
                   packet->data[i]);
        }
        else
        {
            printf("   ");
        }
    }

    printf("| ");

    /*
     * CRC検査
     *
     * CRCエラーの場合は
     * 以降の解析を実施しない。
     *
     * ただし、
     * 通番および時間計測は既に更新済みであり、
     * 表示上の時間軸整合性を維持する。
     *
     * この設計は本プロジェクトで議論し、
     * 「最後に正常受信したパケット基準」
     * ではなく
     * 「全受信パケット基準」
     * を採用したものである。
     */
    if(!check_crc(packet))
    {
        statistics_count_crc_error();

        printf("CRC ERROR\r\n");

        return;
    }

    /*
     * IDLE Packet
     *
     * FF 00 FF
     */
    if(packet->length == 3 &&
       packet->data[0] == 0xFF &&
       packet->data[1] == 0x00 &&
       packet->data[2] == 0xFF)
    {
        statistics_count_idle();

        printf("Idle");
    }

    /*
     * RESET Packet
     *
     * 00 00 00
     */
    else if(packet->data[0] == 0x00)
    {
        statistics_count_reset();

        printf("Reset");
    }

    /*
     * Service Mode Packet
     *
     * サービスモードCV書き込み関連。
     *
     * プログラミング線上で使用される。
     */
    else if(is_service_packet(packet))
    {
        statistics_count_service();

        analyze_service(packet);
    }

    /*
     * OPS Mode Packet
     *
     * Main Track Programming。
     *
     * 本線上でのCV変更に使用される。
     */
    else if(is_ops_packet(packet))
    {
        statistics_count_ops();

        analyze_ops(packet);
    }

    /*
     * Accessory Packet
     *
     * 分岐器・信号機・照明などの
     * アクセサリデコーダ制御。
     */
    else if((packet->data[0] & 0xC0) == 0x80)
    {
        statistics_count_accessory();

        analyze_accessory(packet);
    }

    /*
     * Loco Packet
     *
     * 上記いずれにも該当しない場合は
     * 機関車制御パケットとして扱う。
     *
     * 本システムでは
     * 最終分類先となる。
     */
    else
    {
        statistics_count_loco();

        analyze_loco(packet);
    }

    /*
     * 1パケット表示終了。
     */
    printf("\r\n");
}
