#include "config.h"

bool parse_config(const char *filename, tgn_cfg_stgs *tgn_stgs) {
    config_t cfg;

    config_init(&cfg);

    if (!config_read_file(&cfg, filename)) {
        fprintf(stderr, "Error parsing the configuration file at %s:%d - %s\n",
                config_error_file(&cfg),
                config_error_line(&cfg),
                config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    const char *working_dir;
    if (config_lookup_string(&cfg, "working_dir", &working_dir))
        tgn_stgs->working_dir = strdup(working_dir);

    config_setting_t* cpus = config_lookup(&cfg, "cpus");
    if(!cpus) return false;

    tgn_stgs->cpus[0] = config_setting_get_int_elem(cpus, 0);
    tgn_stgs->cpus[1] = config_setting_get_int_elem(cpus, 1);


    if( !config_lookup_int(&cfg, "max_cnt_mbufs", (int*)&tgn_stgs->max_cnt_mbufs) ||
        !config_lookup_int(&cfg, "num_memory_channels", (int*)&tgn_stgs->num_memory_channels) ||
        !config_lookup_int(&cfg, "nic_queue_size", (int*)&tgn_stgs->nic_queue_size)
            ) return false;

    config_destroy(&cfg);

    return true;
}

//int main() {
//    tgn_cfg_stgs tgn_stgs = {};
//    if (parse_config("config.cfg", &tgn_stgs)) {
//        printf("Working Directory: %s\n", tgn_stgs.working_dir);
//        printf("CPUs: %d, %d\n", tgn_stgs.cpus[0], tgn_stgs.cpus[1]);
//        printf("Max Count Mbufs: %u\n", tgn_stgs.max_cnt_mbufs);
//        printf("Number of Memory Channels: %hu\n", tgn_stgs.num_memory_channels);
//        printf("NIC Queue Size: %hu\n", tgn_stgs.nic_queue_size);
//        free(tgn_stgs.working_dir); // Free dynamically allocated memory
//    } else {
//        printf("Failed to parse configuration.\n");
//    }
//
//    return 0;
//}

