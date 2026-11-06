#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>

/*
 * パケット統計情報
 */
typedef struct
{
    uint32_t total;

    uint32_t reset;
    uint32_t idle;

    uint32_t loco;
    uint32_t accessory;

    uint32_t ops;
    uint32_t service;

    uint32_t crc_error;
    uint64_t elapsed_us;

} PACKET_STATISTICS;

/*
 * 信号統計情報
 */
typedef struct
{
    uint32_t bit0_count;
    uint32_t bit1_count;

    uint64_t bit0_total_us;
    uint64_t bit1_total_us;

} SIGNAL_STATISTICS;

/*
 * 統計データ
 */
extern PACKET_STATISTICS packet_statistics;
extern SIGNAL_STATISTICS signal_statistics;


/*
 * 初期化
 */
void statistics_init(void);

/*
 * 全統計クリア
 */
void statistics_reset(void);

/*
 * カウンタ更新
 */
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

void statistics_count_bit0(void);
void statistics_count_bit1(void);

void statistics_add_bit0_width(
    uint32_t width_us);

void statistics_add_bit1_width(
    uint32_t width_us);

#endif