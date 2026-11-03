#ifndef OPS_ANALYZER_H
#define OPS_ANALYZER_H

#include <stdbool.h>

#include "dcc_packet.h"

bool is_ops_packet(
    const DCC_PACKET *packet);

void analyze_ops(
    const DCC_PACKET *packet);

#endif