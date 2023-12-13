#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/errno.h>

//#include "config/rte_config.h"
//#include "lib/eal/include/rte_errno.h"
//#include "lib/eal/include/rte_eal.h"
//#include "lib/eal/include/rte_lcore.h"
//#include "lib/mempool/rte_mempool.h"
//#include "lib/mbuf/rte_mbuf_core.h"
//#include "lib/mbuf/rte_mbuf.h"
//#include "lib/ethdev/rte_ethdev.h"

#include <rte_config.h>
#include <rte_errno.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_mbuf_core.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>

void dpdk_init(char** params, int sz) {
    if(rte_eal_init(sz, params) < 0) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));
        return;
    } else printf("eal initialized\n");
}

void init_mempool(uint16_t n_pkt) {
    uint16_t socket_id = rte_socket_id();
    char* name = "test-mempool";
    printf("Name: %s, socket_id: %d, n_pkt: %d, rte_mempool_cache_max_size: %d, rte_mbuf_default_buf_size: %d\n", name, socket_id, n_pkt, RTE_MEMPOOL_CACHE_MAX_SIZE, RTE_MBUF_DEFAULT_BUF_SIZE);
    struct rte_mempool* mbuf_pool= rte_pktmbuf_pool_create(name, n_pkt, RTE_MEMPOOL_CACHE_MAX_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, socket_id);
    if(!mbuf_pool) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));

        switch (rte_errno) {
            case E_RTE_NO_CONFIG:
                printf("E_RTE_NO_CONFIG: function could not get pointer to rte_config structure\n");
                break;
            case EEXIST:
                printf("EEXIST: a memzone with the same name already exists\n");
                break;
            case EINVAL:
                printf("EINVAL: cache size provided is too large, or priv_size is not aligned.\n");
                break;
            case ENOSPC:
                printf("ENOSPC: the maximum number of memzones has already been allocated\n");
                break;
            case ENOMEM:
                printf("ENOMEM: no appropriate memory area found in which to create memzone\n");
                break;
            default:
                break;
        }
        return;
    } else printf("Mbuf pool created - name: %s, socket_id: %d, size: %d\n", mbuf_pool->name, mbuf_pool->socket_id, mbuf_pool->size);

}


int main() {
    char* params[4] = {"dpdk_rxtx", "-n 2", "-l 1", "--no-telemetry"};
    dpdk_init(params, 4);

    init_mempool(8192);

}
