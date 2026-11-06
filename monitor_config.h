#ifndef MONITOR_CONFIG_H
#define MONITOR_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    FILTER_ALL,
    FILTER_LOCO,
    FILTER_ACCESSORY,
    FILTER_OPS

} FILTER_MODE;

typedef struct
{
    FILTER_MODE filter;

    bool show_reset;
    bool show_idle;

    uint16_t address_filter;

    uint16_t threshold_us;

    uint8_t preamble_count;

} MONITOR_CONFIG;

extern MONITOR_CONFIG monitor_config;

void monitor_config_init(void);

#endif