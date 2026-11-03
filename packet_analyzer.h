#ifndef PACKET_ANALYZER_H
#define PACKET_ANALYZER_H

#include "dcc_packet.h"

void packet_analyzer_init(void);

void packet_analyzer(
    const DCC_PACKET *packet);

#endif