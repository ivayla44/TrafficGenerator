#ifndef TRAFFIC_GENERATOR_STATS_H
#define TRAFFIC_GENERATOR_STATS_H

#include "../precompiled.h"

typedef struct stats {
    uint64_t cnt_rx_pkts;
    uint64_t cnt_tx_pkts;
    uint64_t cnt_rx_bytes;
    uint64_t cnt_tx_bytes;
    uint64_t cnt_rx_pkts_qfull;
    uint64_t cnt_rx_pkts_nombuf;
    uint64_t cnt_tx_pkts_qfull;
    uint64_t cnt_tx_pkts_nombuf;
    uint64_t cnt_rx_pkts_err;
    uint64_t cnt_tx_pkts_err;
} stats;

typedef struct entry {
    uint32_t gen_idx;
    uint32_t flow_idx;
    uint64_t cnt_pkts;
    uint64_t cnt_bytes;
    uint64_t duration;
} entry;

typedef struct summary_stats {
    stats summary;
    entry** detailed;
} summary_stats;

#endif //TRAFFIC_GENERATOR_STATS_H
