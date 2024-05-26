#ifndef TRAFFIC_GENERATOR_CONFIG_H
#define TRAFFIC_GENERATOR_CONFIG_H

#include "../../precompiled.h"

typedef struct tgn_cfg_stgs {
    char* working_dir;
    int cpus[2];
    uint32_t max_cnt_mbufs;
    uint16_t num_memory_channels;
    uint16_t nic_queue_size;
} tgn_cfg_stgs;

bool parse_config(const char* filename, tgn_cfg_stgs* tgn_stgs);
void print_cfg_stgs(tgn_cfg_stgs* tgn_stgs);

#endif //TRAFFIC_GENERATOR_CONFIG_H
