#include "pcap_loader.h"

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

    return true;
}

struct rte_mbuf* get_packet(FILE* pcap_file, struct rte_mempool* mbuf_pool) {
    struct rte_mbuf* mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if(!mbuf) {
        fprintf(stderr, "Could not allocate mbuf (mbuf pool name: %s)\n", mbuf_pool->name);
        return NULL;
    }

    struct tgn_pcap_pkt_hdr pkt_header;
    if (fread(&pkt_header, sizeof(struct tgn_pcap_pkt_hdr), 1, pcap_file) != 1) {
        if(feof(pcap_file)) {
            return NULL;
        }
        fprintf(stderr, "Could not read pcap packet header\n");
        rte_pktmbuf_free(mbuf);
        return NULL;
    }

    if (pkt_header.caplen <= RTE_MBUF_DEFAULT_BUF_SIZE && pkt_header.caplen == pkt_header.len) {
        if (fread(rte_pktmbuf_mtod(mbuf, char*), pkt_header.caplen, 1, pcap_file) != 1) {
            fprintf(stderr, "Error reading packet data\n");
            rte_pktmbuf_free(mbuf);
            return NULL;
        }

        mbuf->data_len = pkt_header.caplen;
        mbuf->pkt_len = pkt_header.len;
    } else {
        fprintf(stderr, "Packet size exceeds maximum supported size or partial packet.\n");
        rte_pktmbuf_free(mbuf);
        return NULL;
    }

    return mbuf;
}

uint32_t load_packets(struct rte_mbuf** tx_packets, FILE* pcap_file, struct rte_mempool* mbuf_pool) {
    struct rte_mbuf* pkt;
    uint32_t pkt_num = 0;

    for(;;++pkt_num) {
        pkt = get_packet(pcap_file, mbuf_pool);
        if(!pkt) {
            if(feof(pcap_file)) {
                printf("EOF reached. All packets read.\n");
                break;
            }
            fprintf(stderr, "Error reading packet.\n");
            return false;
        }
        tx_packets[pkt_num] = pkt;
    }

    return pkt_num;
}
