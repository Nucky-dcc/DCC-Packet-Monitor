#ifndef LOCO_ANALYZER_H
#define LOCO_ANALYZER_H

#include "dcc_packet.h"

void packet_analyzer_init(void);

void analyze_loco(
    const DCC_PACKET *packet);

#endif