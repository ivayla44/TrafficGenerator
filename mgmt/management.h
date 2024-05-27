#ifndef DPDK_RXTX_MANAGEMENT_H
#define DPDK_RXTX_MANAGEMENT_H

#include "../precompiled.h"
#include "stats.h"

bool save_sum_stats(tgn_stats sum_data);
void print_stats(tgn_stats* summary);

#endif //DPDK_RXTX_MANAGEMENT_H
