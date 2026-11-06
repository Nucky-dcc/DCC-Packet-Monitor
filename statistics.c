#include <stdint.h>
#include <limits.h>

#include "statistics.h"

#define STAT_COUNTER_MAX UINT32_MAX

/*
 * 統計情報
 */
PACKET_STATISTICS packet_statistics;

SIGNAL_STATISTICS signal_statistics;


/*
 * 飽和カウンタ加算
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

/*
 * 初期化
 */
void statistics_init(void)
{
    packet_statistics.total = 0;

    packet_statistics.reset = 0;
    packet_statistics.idle = 0;

    packet_statistics.loco = 0;
    packet_statistics.accessory = 0;

    packet_statistics.ops = 0;
    packet_statistics.service = 0;

    packet_statistics.crc_error = 0;

    packet_statistics.elapsed_us = 0;

    signal_statistics.bit0_count = 0;
    signal_statistics.bit1_count = 0;

    signal_statistics.bit0_total_us = 0;
    signal_statistics.bit1_total_us = 0;
}

/*
 * 統計クリア
 */
void statistics_reset(void)
{
    statistics_init();
}

/*
 * 全パケット数
 */
void statistics_count_total(void)
{
    count_up(
        &packet_statistics.total);
}

/*
 * Reset
 */
void statistics_count_reset(void)
{
    count_up(
        &packet_statistics.reset);
}

/*
 * Idle
 */
void statistics_count_idle(void)
{
    count_up(
        &packet_statistics.idle);
}

/*
 * Loco
 */
void statistics_count_loco(void)
{
    count_up(
        &packet_statistics.loco);
}

/*
 * Accessory
 */
void statistics_count_accessory(void)
{
    count_up(
        &packet_statistics.accessory);
}

/*
 * OPS
 */
void statistics_count_ops(void)
{
    count_up(
        &packet_statistics.ops);
}

/*
 * Service
 */
void statistics_count_service(void)
{
    count_up(
        &packet_statistics.service);
}

/*
 * CRC Error
 */
void statistics_count_crc_error(void)
{
    count_up(
        &packet_statistics.crc_error);
}

void statistics_add_elapsed_us(
    uint32_t elapsed_us)
{
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

void statistics_count_bit0(void)
{
    count_up(
        &signal_statistics.bit0_count);
}

void statistics_count_bit1(void)
{
    count_up(
        &signal_statistics.bit1_count);
}

void statistics_add_bit0_width(
    uint32_t width_us)
{
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

void statistics_add_bit1_width(
    uint32_t width_us)
{
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

