#ifndef TRAFFIC_GENERATOR_MBUF_POOL_H
#define TRAFFIC_GENERATOR_MBUF_POOL_H

#include "../../precompiled.h"
#include "../../utils/file_utils.h"
#include "../../app/cfg/config.h"

struct rte_mempool* init_mempool(uint16_t socket_id, tgn_cfg_stgs *cfg);


#endif //TRAFFIC_GENERATOR_MBUF_POOL_H
