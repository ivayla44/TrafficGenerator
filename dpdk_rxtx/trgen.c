#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/errno.h>

//#include "config/rte_config.h"
//#include "lib/eal/include/rte_errno.h"
//#include "lib/eal/include/rte_eal.h"
//#include "lib/eal/include/rte_lcore.h"
//#include "lib/mempool/rte_mempool.h"
//#include "lib/mbuf/rte_mbuf_core.h"
//#include "lib/mbuf/rte_mbuf.h"
//#include "lib/ethdev/rte_ethdev.h"
//#include "lib/net/rte_ether.h"
//#include "lib/eal/include/rte_bitops.h"

#include <rte_config.h>
#include <rte_errno.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_mbuf_core.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_bitops.h>

// TODO: add these to a header file (dont feel like rebuilding rn :))
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4

struct pcap_file_header {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;	/* gmt to local correction; this is always 0 */
    uint32_t sigfigs;	/* accuracy of timestamps; this is always 0 */
    uint32_t snaplen;	/* max length saved portion of each pkt */
    uint32_t linktype;	/* data link type (LINKTYPE_*) */
};

struct pcap_pkt_header {
    uint32_t sec;
    uint32_t usec;
    uint32_t caplen;
    uint32_t len;
};


// TODO: sizes based on what? (these are from examples from the dpdk documentation)
#define RTE_RX_DESC_DEFAULT 1024
#define RTE_TX_DESC_DEFAULT 1024

#define MAX_PACKET_SIZE 2048 // same as RTE_MBUF_DEFAULT_BUF_SIZE?
#define MAX_PKT_BURST 32

// comments about this: ln. 101
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

bool init_rx_tx_queues(uint16_t eth_port_id, uint16_t socket_id, struct rte_mempool* mbuf_pool) {

    struct rte_eth_dev_info dev_info;
    if(rte_eth_dev_info_get(eth_port_id, &dev_info) != 0) {
        fprintf(stderr, "Unable to get device info.\n");
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

    printf("DPDK port initialized\n");
    return true;
}

struct rte_ether_addr* get_mac_addr(uint16_t eth_port_id) {
    struct rte_ether_addr* mac = malloc(sizeof(struct rte_ether_addr));
    if(!mac) {
        fprintf(stderr, "Unable to allocate memory for mac address.\n");
        return NULL;
    }
    if(!rte_eth_macaddr_get(eth_port_id, mac)) {
        fprintf(stderr, "Unable to get mac address (port id: %d)\n", eth_port_id);
        return NULL;
    }
    return mac;
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

bool check_pcap(FILE* pcap_file) {
    struct pcap_file_header header;
    if(fread(&header, sizeof(struct pcap_file_header), 1, pcap_file) != 1) {
        fprintf(stderr, "Could not read file header.\n");
        return false;
    }

    uint32_t pcap_magic = 0xA1B2C3D4;
    if ((header.magic != pcap_magic) || (header.version_major != PCAP_VERSION_MAJOR) || (header.version_minor != PCAP_VERSION_MINOR)) {
        fprintf(stderr, "Invalid file - magic: %x, version_major: %d, version_minor: %d.\n", header.magic, header.version_major, header.version_minor);
        return false;
    }

    printf("Valid pcap file.\n");
    return true;
}

struct rte_mbuf* get_packet(FILE* pcap_file, struct rte_mempool* mbuf_pool) {
    struct rte_mbuf* mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if(!mbuf) {
        fprintf(stderr, "Could not allocate mbuf (mbuf pool name: %s)\n", mbuf_pool->name);
        return NULL;
    }

    struct pcap_pkt_header pkt_header;
    if (fread(&pkt_header, sizeof(struct pcap_pkt_header), 1, pcap_file) != 1) {
        if(feof(pcap_file)) {
            return NULL;
        }
        fprintf(stderr, "Could not read pcap packet header\n");
        rte_pktmbuf_free(mbuf);
        return NULL;
    }

    if (pkt_header.caplen <= MAX_PACKET_SIZE) {
        if (fread(rte_pktmbuf_mtod(mbuf, char*), pkt_header.caplen, 1, pcap_file) != 1) {
            fprintf(stderr, "Error reading packet data\n");
            rte_pktmbuf_free(mbuf);
            return NULL;
        }

        mbuf->data_len = pkt_header.caplen;
        mbuf->pkt_len = pkt_header.len;

        printf("Packet - packet_data_len: %d, packet_len: %d\n", mbuf->data_len, mbuf->pkt_len);
    } else {
        fprintf(stderr, "Packet size exceeds maximum supported size\n");
        rte_pktmbuf_free(mbuf);
        return NULL;
    }

    return mbuf;
}

struct rte_mbuf* load_packets(FILE* pcap_file, struct rte_mempool* mbuf_pool) {
    struct rte_mbuf* head = get_packet(pcap_file, mbuf_pool);
    if(!head) {
        fprintf(stderr, "Error reading initial packet.\n");
        fclose(pcap_file);
        return NULL;
    }
    struct rte_mbuf* curr = head;
    struct rte_mbuf* prev = NULL;

    while(!feof(pcap_file)) {
        if(prev) {
            curr = get_packet(pcap_file, mbuf_pool);
            if(!curr) {
                if(feof(pcap_file)) {
//                    TODO: this works but def not a good fix, need to figure out a better way to do this
                    printf("EOF reached. All packets read.\n");
                    break;
                }
                fprintf(stderr, "Error reading next packet.\n");
                fclose(pcap_file);
                return NULL;
            }
            prev->next = curr;
            head->nb_segs++;
        }
        head->pkt_len += curr->data_len;
        prev = curr;
    }

    return head;
}

//void prep_tx_packets(struct rte_mbuf** tx_packets, struct rte_mbuf* head, struct rte_ether_addr* dest_addr, uint16_t eth_port_id) {
//    struct rte_ether_addr* src_addr = get_mac_addr(eth_port_id);
//    if(!src_addr) {
//        fprintf(stderr, "Couldn't get src mac address.\n");
//        return;
//    }
//
//    struct rte_mbuf* pkt = head;
//
//    for(uint32_t pkt_indx = 0; pkt_indx < head->nb_segs; pkt = pkt->next, ++pkt_indx) {
//        struct rte_ether_hdr *ethdr;
//        ethdr = (struct rte_ether_hdr *)rte_pktmbuf_prepend(pkt, (uint16_t)sizeof(*ethdr));
//
//        rte_ether_addr_copy(dest_addr, &ethdr->dst_addr);
//        rte_ether_addr_copy(src_addr, &ethdr->src_addr);
//
//        tx_packets[pkt_indx] = pkt;
//    }
//
//    free(src_addr);
//}

int main() {
    char* params[4] = {"dpdk_rxtx", "-n 2", "-l 1", "--no-telemetry"};
    dpdk_init(params, 4);

    uint16_t socket_id = rte_socket_id();

    struct rte_mempool* mbuf_pool = init_mempool(8192, socket_id);

    if(log_avail_eth_ports() == (uintptr_t)NULL) {
        fprintf(stderr, "No available ports. Exiting.\n");
        exit(1);
    }

    uint16_t eth_port_id = rte_eth_find_next(0);
    printf("Eth port ID: %d\n", eth_port_id);

    if(!init_rx_tx_queues(eth_port_id, socket_id, mbuf_pool)) {
        fprintf(stderr,"Cannot initialize dpdk port. Exiting.\n");
        exit(2);
    }

    start_port(eth_port_id);

    FILE* pcap_file = fopen("./files/tg-test.pcap", "r");
    if(!pcap_file) {
        fprintf(stderr, "Cannot open file. Exiting.\n");
        exit(3);
    }

    if(!check_pcap(pcap_file)) {
        fclose(pcap_file);
        exit(4);
    }

    struct rte_mbuf* head = load_packets(pcap_file, mbuf_pool);

    char* mac_addr_str = "e4:8d:8c:20:fb:bb";
    struct rte_ether_addr dest_addr;
    if(rte_ether_unformat_addr(mac_addr_str, &dest_addr) != 0) {
        fprintf(stderr, "Cannot parse mac addr string to rte_ether_addr struct.\n");
        exit(3);
    }

//    struct rte_mbuf* tx_packets[MAX_PKT_BURST];
//    prep_tx_packets(tx_packets, head, &dest_addr, eth_port_id);

}
