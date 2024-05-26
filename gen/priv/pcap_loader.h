#ifndef TRAFFIC_GENERATOR_PCAP_LOADER_H
#define TRAFFIC_GENERATOR_PCAP_LOADER_H

#include "../../precompiled.h"

typedef struct tgn_pcap_pkt_hdr {
    uint32_t sec;
    uint32_t usec;
    uint32_t caplen;
    uint32_t len;
}tgn_pcap_pkt_hdr;

bool check_pcap(FILE* pcap_file);
struct rte_mbuf* get_packet(FILE* pcap_file, struct rte_mempool* mbuf_pool);
uint32_t load_packets(struct rte_mbuf** tx_packets, FILE* pcap_file, struct rte_mempool* mbuf_pool);

#endif //TRAFFIC_GENERATOR_PCAP_LOADER_H
