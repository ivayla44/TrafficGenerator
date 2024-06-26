#include "precompiled.h"

#include "gen/priv/pcap_loader.h"
#include "gen/priv/mbuf_pool.h"
#include "gen/priv/eth_dev.h"
#include "app/cfg/config.h"
#include "mgmt/gen_config.h"
#include "mgmt/management.h"

#define MAX_PKT_BURST 32

bool dpdk_init(char** params, int sz) {
    if(rte_eal_init(sz, params) < 0) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));
        return false;
    } return true;
}


void prep_tx_packets(struct rte_mbuf** tx_packets, uint32_t pkt_num, const struct rte_ether_addr* src_mac_addr, const struct rte_ether_addr* dest_mac_addr, uint16_t eth_port_id) {
    for(uint32_t pkt_indx = 0; pkt_indx < pkt_num; ++pkt_indx) {
        struct rte_mbuf* pkt = tx_packets[pkt_indx];

        struct rte_ether_hdr* eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr*);

//        fprintf(stderr, "Before Pkt:%p Eh:%p SrcEther:" RTE_ETHER_ADDR_PRT_FMT " DstEther:" RTE_ETHER_ADDR_PRT_FMT "\n",
//                pkt, eth_hdr, RTE_ETHER_ADDR_BYTES(&eth_hdr->src_addr), RTE_ETHER_ADDR_BYTES(&eth_hdr->dst_addr));

        rte_ether_addr_copy(src_mac_addr, &eth_hdr->src_addr);
        rte_ether_addr_copy(dest_mac_addr, &eth_hdr->dst_addr);

//        fprintf(stderr, "After Pkt:%p Eh:%p SrcEther:" RTE_ETHER_ADDR_PRT_FMT " DstEther:" RTE_ETHER_ADDR_PRT_FMT "\n",
//                pkt, eth_hdr, RTE_ETHER_ADDR_BYTES(&eth_hdr->src_addr), RTE_ETHER_ADDR_BYTES(&eth_hdr->dst_addr));

        struct rte_ipv4_hdr* ipv4_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_ipv4_hdr*, sizeof(struct rte_ether_hdr));

        ipv4_hdr->hdr_checksum = 0;

        uint64_t flags = RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM |
                         RTE_MBUF_F_TX_TCP_CKSUM | RTE_MBUF_F_TX_UDP_CKSUM;

        pkt->ol_flags |= flags;
        pkt->l2_len = RTE_ETHER_HDR_LEN;
        pkt->l3_len = ((ipv4_hdr->version_ihl & 0x0F) * 4u);
    }
}

uint32_t rx_loop(uint16_t eth_port_id, uint16_t nb_pkts, struct rte_mbuf** tx_packets, uint16_t duration_seconds, tgn_stats* tgn_summary) {
    struct rte_mbuf* rx_packets[MAX_PKT_BURST];
    int nb_rx, cnt = 0;
    uint32_t pkts = 0;

    time_t start_time = time(NULL);

    while(true) {
        if (difftime(time(NULL), start_time) >= duration_seconds) {
            break;
        }

        if(!(++cnt % 1000000)) {
            uint16_t sent = rte_eth_tx_burst(eth_port_id, 0, tx_packets, nb_pkts);
            if(sent < nb_pkts) {
                struct rte_eth_stats tmp = {};
                if(!rte_eth_stats_get(eth_port_id, &tmp)) {
                    fprintf(stderr,
                            "Successfully transmitted packets: %lu\nFailed transmitted packets: %lu\n",
                            tmp.opackets, tmp.oerrors);
                }
                return EXIT_FAILURE;
            }

        }

        nb_rx = rte_eth_rx_burst(eth_port_id, 0, rx_packets, nb_pkts);

        if(nb_rx) {
            for(uint32_t pkt_indx = 0; pkt_indx < nb_rx; pkt_indx++) {
                fprintf(stdout, "\nReceived packet: %d, pkt_len: %u, data_len: %u, pool: %s.\n\n", pkts, rx_packets[pkt_indx]->pkt_len, rx_packets[pkt_indx]->data_len, rx_packets[pkt_indx]->pool->name);
                pkts++;
            }
        }
    }

    struct rte_eth_stats rte_summary = {};
    if(!rte_eth_stats_get(eth_port_id, &rte_summary)) {
        tgn_summary->cnt_rx_pkts = rte_summary.ipackets;
        tgn_summary->cnt_tx_pkts = rte_summary.opackets;
        tgn_summary->cnt_rx_bytes = rte_summary.ibytes;
        tgn_summary->cnt_tx_bytes = rte_summary.obytes;
        tgn_summary->cnt_rx_pkts_qfull = rte_summary.imissed;
        tgn_summary->cnt_rx_pkts_nombuf = rte_summary.rx_nombuf;
        tgn_summary->cnt_rx_pkts_err = rte_summary.ierrors;
        tgn_summary->cnt_tx_pkts_err = rte_summary.oerrors;
    }
    else {
        fprintf(stderr, "Error loading summary stats.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    char* params[4] = {"dpdk_rxtx", "-n 2", "-l 1", "--no-telemetry"};
    dpdk_init(params, 4);

    uint16_t socket_id = rte_socket_id();


    tgn_cfg_stgs tgn_stgs = {};
    if(parse_config("./files/config.cfg", &tgn_stgs)) {
        print_cfg_stgs(&tgn_stgs);
    }
    else {
        fprintf(stderr, "Failed to parse configuration.\n");
        return EXIT_FAILURE;
    }

    struct rte_mempool* mbuf_pool = init_mempool(socket_id, &tgn_stgs);

    if(!log_avail_eth_ports()) {
        fprintf(stderr, "No available ports. Exiting.\n");
        return EXIT_FAILURE;
    }

    uint16_t eth_port_id = rte_eth_find_next(0);

    if(!init_rx_tx_queues(eth_port_id, socket_id, mbuf_pool)) {
        fprintf(stderr,"Cannot initialize dpdk port. Exiting.\n");
        return EXIT_FAILURE;
    }

    start_port(eth_port_id);


    FILE* yaml_file = fopen("./files/yaml_cfg.yaml", "r");

    if(!yaml_file) {
        fprintf(stderr, "Cannot open file.\n");
        return EXIT_FAILURE;
    }

    tgn_cfg cfg = {};

    if(!prs_cfg(yaml_file, &cfg)) {
        fprintf(stderr, "YAML configuration parsing error.\n");
        return EXIT_FAILURE;
    }

    print_cfg(&cfg, 2);


    fclose(yaml_file);

    FILE* pcap_file = fopen("./files/2.pcap", "r");
    if(!pcap_file) {
        fprintf(stderr, "Cannot open file. Exiting.\n");
        return EXIT_FAILURE;
    }

    if(!check_pcap(pcap_file)) {
        fclose(pcap_file);
        return EXIT_FAILURE;
    }

    struct rte_mbuf* tx_packets[MAX_PKT_BURST];

    uint32_t pkt_num = load_packets(tx_packets, pcap_file, mbuf_pool);
    if(!pkt_num) {
        fprintf(stderr, "Error loading packets. Exiting.\n");
        return EXIT_FAILURE;
    }

    struct rte_ether_addr src_mac_addr;
    if(rte_eth_macaddr_get(eth_port_id, &src_mac_addr) != 0) {
        fprintf(stderr, "Unable to get mac address (port id: %d)\n", eth_port_id);
        return EXIT_FAILURE;
    }

    char* dest_mac_addr_str = cfg.dut_ether_addr;
    fprintf(stdout, "DUT: %s\n", cfg.dut_ether_addr);
    struct rte_ether_addr dest_mac_addr;
    if(rte_ether_unformat_addr(dest_mac_addr_str, &dest_mac_addr) != 0) {
        fprintf(stderr, "Cannot parse mac addr string to rte_ether_addr struct.\n");
        return EXIT_FAILURE;
    }

    prep_tx_packets(tx_packets, pkt_num, &src_mac_addr, &dest_mac_addr, eth_port_id);

    tgn_stats summary = {};
    rx_loop(eth_port_id, pkt_num, tx_packets, cfg.duration_secs, &summary);

    print_stats(&summary);

    if(!save_sum_stats(summary)) {
        fprintf(stdout, "Stats saved.\n");
    }
    else {
        fprintf(stderr, "Stats saving issue.\n");
    }

}
