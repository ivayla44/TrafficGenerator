#include "eth_dev.h"

uint16_t log_avail_eth_ports() {
    uint16_t num_ports = rte_eth_dev_count_avail();
    if(!num_ports) {
        fprintf(stderr, "No available eth ports.\n");
        return false;
    }
    return num_ports;
}

bool init_rx_tx_queues(uint16_t eth_port_id, uint16_t socket_id, struct rte_mempool* mbuf_pool) {

    struct rte_eth_dev_info dev_info;
    if(rte_eth_dev_info_get(eth_port_id, &dev_info) != 0) {
        fprintf(stderr, "Unable to get device info.\n");
        return false;
    }
    printf("Max rx queues: %d; Max tx queues: %d\n", dev_info.max_rx_queues, dev_info.max_tx_queues);


    struct rte_eth_conf port_conf = {};

    port_conf.rx_adv_conf.rss_conf.rss_hf = VALID_RSS_HF;


    port_conf.rxmode.offloads &= ~RTE_ETH_RX_OFFLOAD_KEEP_CRC;

    if ((dev_info.rx_offload_capa & RTE_ETH_RX_OFFLOAD_CHECKSUM) == RTE_ETH_RX_OFFLOAD_CHECKSUM) {
        port_conf.rxmode.offloads |= RTE_ETH_RX_OFFLOAD_CHECKSUM;
        printf("RX checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable RX checksum offload\n");
        return false;
    }

    if ((dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) == RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) {
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_IPV4_CKSUM;
        printf("TX IPv4 checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable TX IPv4 checksum offload\n");
        return false;
    }

    uint64_t capa = (RTE_ETH_TX_OFFLOAD_TCP_CKSUM | RTE_ETH_TX_OFFLOAD_UDP_CKSUM);
    if((dev_info.tx_offload_capa & capa) == capa) {
        port_conf.txmode.offloads |= capa;
        printf("TX TCP/UDP checksum offload enabled\n");
    } else {
        fprintf(stderr, "Cannot enable TX TCP/UDP checksum offload\n");
        return false;
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

    printf("DPDK port initialized\n");

    return true;
}

void start_port(uint16_t eth_port_id) {
    if (rte_eth_dev_start(eth_port_id) < 0) {
        fprintf(stderr, "Failed to start DPDK port; (Err value: %s)\n", rte_strerror(rte_errno));
        return;
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
        return;
    }

    char* mode = (link.link_duplex == RTE_ETH_LINK_FULL_DUPLEX) ? "full-duplex" : "half-duplex";

    printf("Successfully started DPDK port: %d. Speed: %u Mbps. Mode: %s\n", eth_port_id, link.link_speed, mode);
}

