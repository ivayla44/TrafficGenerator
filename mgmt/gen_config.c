#include "gen_config.h"

bool prs_cfg(FILE* yaml_file, tgn_cfg* cfg) {
    uint32_t count;

    struct fy_document *fyd = fy_document_build_from_fp(NULL, yaml_file);
    if (!fyd) {
        fprintf(stderr, "failed to build document");
        return false;
    }

    count = fy_document_scanf(fyd,
                              "/duration_secs %llu "
                              "/dut_ether_addr %s",
                              &cfg->duration_secs, cfg->dut_ether_addr);

    if (count != 2) {
        fprintf(stderr, "failed to read document");
        return false;
    }

    struct fy_node *captures_node = fy_node_by_path(fy_document_root(fyd), "/captures", -1, 0);
    if (!captures_node) {
        fprintf(stderr, "captures node not found");
        fy_document_destroy(fyd);
        return false;
    }

    int captures_count = fy_node_sequence_item_count(captures_node);

    cfg->captures = malloc(sizeof(tgn_cptr) * captures_count);
    if(!cfg->captures) {
        fprintf(stderr, "Cannot allocate memory for captures array.\n");
        return false;
    }

    for (int i = 0; i < captures_count; ++i) {
        struct fy_node *capture_node = fy_node_sequence_get_by_index(captures_node, i);

        strcpy(cfg->captures[i].name, fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "name", -1));
        cfg->captures[i].bursts = strtoull(fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "burst", -1), NULL, 10);
        cfg->captures[i].fps = strtoull(fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "fps", -1), NULL, 10);
        cfg->captures[i].ipg = fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "ipg", -1) != NULL ?
                               strtoull(fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "ipg", -1), NULL, 10) :
                               0;
        strcpy(cfg->captures[i].cln_ips, fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "cln_ips", -1));
        strcpy(cfg->captures[i].srv_ips, fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "srv_ips", -1));
        cfg->captures[i].cln_port = fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "cln_port", -1) != NULL ?
                                    strtoull(fy_node_mapping_lookup_scalar0_by_simple_key(capture_node, "cln_port", -1), NULL, 10) :
                                    0;
    }

    fy_document_destroy(fyd);
    return true;
}

void print_cfg(tgn_cfg* cfg, uint32_t nfile) {
    fprintf(stdout, "duration_secs: %llu, dut_ether_addr %s\n", cfg->duration_secs, cfg->dut_ether_addr);

    for(int i = 0; i < nfile; i++) {
            fprintf(stdout, "name: %s; bursts: %llu; fps: %llu; ipg: %llu; cln_ips: %s; srv_ips: %s; cln_port %llu\n",
                    cfg->captures[i].name,
                    cfg->captures[i].bursts,
                    cfg->captures[i].fps,
                    cfg->captures[i].ipg,
                    cfg->captures[i].cln_ips,
                    cfg->captures[i].srv_ips,
                    cfg->captures[i].cln_port
            );
    }
}
