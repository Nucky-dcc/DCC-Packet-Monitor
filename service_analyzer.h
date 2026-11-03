#ifndef SERVICE_ANALYZER_H
#define SERVICE_ANALYZER_H

#include "dcc_packet.h"

bool is_service_packet(
    const DCC_PACKET *packet);

void analyze_service(
    const DCC_PACKET *packet);

#endif