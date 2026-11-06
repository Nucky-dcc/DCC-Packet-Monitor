#include "monitor_config.h"

MONITOR_CONFIG monitor_config;

void monitor_config_init(void)
{
    monitor_config.filter = FILTER_ALL;

    monitor_config.show_reset = true;
    monitor_config.show_idle  = true;

    monitor_config.address_filter = 0;

    monitor_config.threshold_us = 87;
    monitor_config.preamble_count = 14;
}