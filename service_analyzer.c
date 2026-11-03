#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "service_analyzer.h"

static uint8_t current_page = 1;

bool is_service_packet(
    const DCC_PACKET *packet)
{
    /*
     * 仮実装
     * あとで判定を追加
     */
    (void)packet;

    return false;
}

void analyze_service(
    const DCC_PACKET *packet)
{
    uint8_t inst;

    inst = packet->data[0];

    printf("Prog ");

    (void)inst;

    printf("Unknown");
}