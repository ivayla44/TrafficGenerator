#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/errno.h>

#include "config/rte_config.h"
#include "lib/eal/include/rte_errno.h"
#include "lib/eal/include/rte_eal.h"
#include "lib/eal/include/rte_lcore.h"
#include "lib/mempool/rte_mempool.h"
#include "lib/mbuf/rte_mbuf_core.h"
#include "lib/mbuf/rte_mbuf.h"
#include "lib/ethdev/rte_ethdev.h"
#include "lib/net/rte_ether.h"
#include "lib/eal/include/rte_bitops.h"

//#include <rte_config.h>
//#include <rte_errno.h>
//#include <rte_eal.h>
//#include <rte_lcore.h>
//#include <rte_mempool.h>
//#include <rte_mbuf_core.h>
//#include <rte_mbuf.h>
//#include <rte_ethdev.h>
//#include <rte_ether.h>
//#include <rte_bitops.h>


// TODO: sizes based on what? (these are from examples from the dpdk documentation)
#define RTE_RX_DESC_DEFAULT 1024
#define RTE_TX_DESC_DEFAULT 1024

// comments about this: ln. 78
#define VALID_RSS_HF 0x38d34


void dpdk_init(char** params, int sz) {
    if(rte_eal_init(sz, params) < 0) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));
        return;
    } else printf("eal initialized\n");
}

struct rte_mempool* init_mempool(uint16_t n_pkt, uint16_t socket_id) {
    char* name = "test-mempool";

    struct rte_mempool* mbuf_pool= rte_pktmbuf_pool_create(name, n_pkt, RTE_MEMPOOL_CACHE_MAX_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, socket_id);
    if(!mbuf_pool) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));
        return NULL;
    }
    printf("Mbuf pool created - name: %s, socket_id: %d, size: %d\n", mbuf_pool->name, mbuf_pool->socket_id, mbuf_pool->size);
    return mbuf_pool;
}

uint16_t log_avail_eth_ports() {
    uint16_t num_ports = rte_eth_dev_count_avail();
    if(!num_ports) {
        fprintf(stderr, "No available eth ports.\n");
        return (uintptr_t)NULL;
    }
    printf("Number of available Ethernet ports: %u\n", num_ports);
}

bool init_rx_tx_queues(uint32_t eth_port_id, uint16_t socket_id, struct rte_mempool* mbuf_pool) {

    struct rte_eth_dev_info dev_info;
    if(rte_eth_dev_info_get(eth_port_id, &dev_info) != 0) {
        fprintf(stderr, "Unable to get device info.");
        return false;
    }
    printf("Max rx queues: %d; Max tx queues: %d\n", dev_info.max_rx_queues, dev_info.max_tx_queues);


    struct rte_eth_conf port_conf;

    // TODO: (figure this out) works like this so i'll leave it for now, def isn't a solution but idk what to do w it
    // error returned in rte_eth_dev_configure call:
    // < Ethdev port_id=0 invalid rss_hf: 0x7ffdbeda36f0, valid value: 0x38d34 >
    port_conf.rx_adv_conf.rss_conf.rss_hf = VALID_RSS_HF;


    // TODO: read up on how this works
    // When this feature is supported and enabled, incoming packets can be checked for correctness in the NIC itself,
    // which can result in improved network performance and reduced CPU utilization for the host system.
    port_conf.rxmode.offloads &= ~RTE_ETH_RX_OFFLOAD_KEEP_CRC;

    if ((dev_info.rx_offload_capa & RTE_ETH_RX_OFFLOAD_CHECKSUM) == RTE_ETH_RX_OFFLOAD_CHECKSUM) {
        port_conf.rxmode.offloads |= RTE_ETH_RX_OFFLOAD_CHECKSUM;
        printf("RX checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable RX checksum offload\n");
    }

    if ((dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) == RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) {
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_IPV4_CKSUM;
        printf("TX IPv4 checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable TX IPv4 checksum offload\n");
    }

    uint64_t capa = (RTE_ETH_TX_OFFLOAD_TCP_CKSUM | RTE_ETH_TX_OFFLOAD_UDP_CKSUM);
    if((dev_info.tx_offload_capa & capa) == capa) {
        port_conf.txmode.offloads |= capa;
        printf("TX TCP/UDP checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable TX TCP/UDP checksum offload\n");
    }


    if(rte_eth_dev_configure(eth_port_id, 1, 1, &port_conf) != 0) {
        fprintf(stderr, "Cannot configure device: 1 rx and 1 tx queue.\n");
        return false;
    }


    uint16_t rx_qs_desc = RTE_RX_DESC_DEFAULT;
    uint16_t tx_qs_desc = RTE_TX_DESC_DEFAULT;

    if(rte_eth_dev_adjust_nb_rx_tx_desc(eth_port_id, &rx_qs_desc, &tx_qs_desc) != 0) {
        fprintf(stderr, "Unable to setup queues (rx queue desc sz: %d, tx queue desc sz: %d)\n", rx_qs_desc, tx_qs_desc);
        return false;
    }


    struct rte_eth_rxconf rxconf = dev_info.default_rxconf;
    rxconf.offloads = port_conf.rxmode.offloads;
    if(rte_eth_rx_queue_setup(eth_port_id, 0, rx_qs_desc, socket_id, &rxconf, mbuf_pool) != 0) {
        fprintf(stderr, "Unable to setup RX queue (port id: %d, queue id: 0, desc: %d, socket id: %d)\n", eth_port_id, rx_qs_desc, socket_id);
        return false;
    }

    struct rte_eth_txconf txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    if(rte_eth_tx_queue_setup(eth_port_id, 0, tx_qs_desc, socket_id, &txconf) != 0) {
        fprintf(stderr, "Unable to setup TX queue (port id: %d, queue id: 0, desc: %d, socket id: %d)\n", eth_port_id, tx_qs_desc, socket_id);
        return false;
    }


    struct rte_ether_addr mac;
    rte_eth_macaddr_get(eth_port_id, &mac);
    printf("DPDK port initialized\n");
    return true;
}

void start_port(uint32_t eth_port_id) {
    if (rte_eth_dev_start(eth_port_id) < 0) {
        fprintf(stderr, "Failed to start DPDK port; (Err value: %s)\n", rte_strerror(rte_errno));
    }

    uint32_t check_cnt = 100, sleep_us  = 100000;

    struct rte_eth_link link = {};
    for (uint32_t i = 0; (i < check_cnt) && !link.link_status; ++i) {
        usleep(sleep_us);
        rte_eth_link_get_nowait(eth_port_id, &link);
    }
    if (!link.link_status) {
        rte_eth_dev_stop(eth_port_id);
        fprintf(stderr, "Failed to bring up DPDK port\n");
    }

    char* mode = (link.link_duplex == RTE_ETH_LINK_FULL_DUPLEX) ? "full-duplex" : "half-duplex";
    printf("Successfully started DPDK port: %d. Speed: %u Mbps. Mode: %s\n", eth_port_id, link.link_speed, mode);
}

int main() {
    char* params[4] = {"dpdk_rxtx", "-n 2", "-l 1", "--no-telemetry"};
    dpdk_init(params, 4);

    uint16_t socket_id = rte_socket_id();

    struct rte_mempool* mbuf_pool = init_mempool(8192, socket_id);

    if(log_avail_eth_ports() == (uintptr_t)NULL) exit(1);

    uint16_t eth_port_id = rte_eth_find_next(0);
    printf("Eth port ID: %d\n", eth_port_id);

    if(!init_rx_tx_queues(eth_port_id, socket_id, mbuf_pool)) {
        fprintf(stderr,"Cannot initialize dpdk port. Exiting.\n");
        exit(2);
    }

    start_port(eth_port_id);

}
