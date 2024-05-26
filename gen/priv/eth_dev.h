#ifndef TRAFFIC_GENERATOR_ETH_DEV_H
#define TRAFFIC_GENERATOR_ETH_DEV_H

#include "../../precompiled.h"

#define VALID_RSS_HF 0x38d34

#define RTE_RX_DESC_DEFAULT 1024
#define RTE_TX_DESC_DEFAULT 1024

uint16_t log_avail_eth_ports();
bool init_rx_tx_queues(uint16_t eth_port_id, uint16_t socket_id, struct rte_mempool* mbuf_pool);
void start_port(uint16_t eth_port_id);

#endif //TRAFFIC_GENERATOR_ETH_DEV_H
