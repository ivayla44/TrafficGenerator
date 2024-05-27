#ifndef TRAFFIC_GENERATOR_STATS_H
#define TRAFFIC_GENERATOR_STATS_H

#include "../precompiled.h"

typedef struct tgn_stats {
    uint64_t cnt_rx_pkts;
    uint64_t cnt_tx_pkts;
    uint64_t cnt_rx_bytes;
    uint64_t cnt_tx_bytes;
    uint64_t cnt_rx_pkts_qfull;
    uint64_t cnt_rx_pkts_nombuf;
    uint64_t cnt_rx_pkts_err;
    uint64_t cnt_tx_pkts_err;
} tgn_stats;

#endif //TRAFFIC_GENERATOR_STATS_H
