#ifndef TRAFFIC_GENERATOR_GEN_CONFIG_H
#define TRAFFIC_GENERATOR_GEN_CONFIG_H

#include "../precompiled.h"

#define SIZEOF_MAC_ADDR 17
#define SIZEOF_IPV4_ADDR 18

typedef struct cptr {
    char name[100];
    uint64_t bursts;
    uint64_t fps;
    uint64_t ipg;
    char cln_ips[SIZEOF_IPV4_ADDR];
    char srv_ips[SIZEOF_IPV4_ADDR];
    uint64_t cln_port;

}tgn_cptr;

typedef struct cfg {
    uint64_t duration_secs;
    char dut_ether_addr[SIZEOF_MAC_ADDR];
    tgn_cptr* captures;
}tgn_cfg;

bool prs_cfg(FILE* yaml_file, tgn_cfg* cfg);

#endif //TRAFFIC_GENERATOR_GEN_CONFIG_H
