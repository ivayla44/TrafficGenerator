#include "mbuf_pool.h"

struct rte_mempool* init_mempool(uint16_t socket_id, tgn_cfg_stgs *cfg) {
    uint32_t hgpage_sz = 2 * 1024 * 1024;
    uint32_t other_fds = 1024;
    uint16_t cache_sz = RTE_MEMPOOL_CACHE_MAX_SIZE;
    uint16_t priv_sz = 0;
    uint16_t mbuf_sz = RTE_MBUF_DEFAULT_BUF_SIZE;
    char* name = "tgn_mbuf_pool";

    uint64_t cnt_fds = other_fds + (cfg->max_cnt_mbufs * mbuf_sz / hgpage_sz);
    if(!set_max_fds(cnt_fds)) {
        fprintf(stderr, "Exceeding sys limits - err: %d, value: %s\n", errno, strerror(errno));
        return NULL;
    }

    struct rte_mempool* mbuf_pool = rte_pktmbuf_pool_create(name, cfg->max_cnt_mbufs, cache_sz, priv_sz, mbuf_sz, socket_id);
    if(!mbuf_pool) {
        fprintf(stderr, "Errno: %d\nValue: %s\n", rte_errno, rte_strerror(rte_errno));
        return NULL;
    }
    return mbuf_pool;
}
