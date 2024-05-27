#include "../precompiled.h"
#include "stats.h"

bool save_sum_stats(tgn_stats sum_data) {
    printf("in save stats\n");
    cJSON *summary_json = cJSON_CreateObject();

    printf("create obj\n");

    cJSON_AddItemToObject(summary_json, "cnt_rx_pkts", cJSON_CreateNumber(sum_data.cnt_rx_pkts));
    cJSON_AddItemToObject(summary_json, "cnt_tx_pkts", cJSON_CreateNumber(sum_data.cnt_tx_pkts));
    cJSON_AddItemToObject(summary_json, "cnt_rx_bytes", cJSON_CreateNumber(sum_data.cnt_rx_bytes));
    cJSON_AddItemToObject(summary_json, "cnt_tx_bytes", cJSON_CreateNumber(sum_data.cnt_tx_bytes));
    cJSON_AddItemToObject(summary_json, "cnt_rx_pkts_qfull", cJSON_CreateNumber(sum_data.cnt_rx_pkts_qfull));
    cJSON_AddItemToObject(summary_json, "cnt_rx_pkts_nombuf", cJSON_CreateNumber(sum_data.cnt_rx_pkts_nombuf));
    cJSON_AddItemToObject(summary_json, "cnt_rx_pkts_err", cJSON_CreateNumber(sum_data.cnt_rx_pkts_err));


    char *json_string = cJSON_Print(summary_json);

    printf("create str\n");


    printf("%s\n", json_string);

    FILE *file = fopen("./files/summary_stats.json", "w");
    if (file == NULL) {
        perror("Failed to open file");
        cJSON_Delete(summary_json);
        free(json_string);
        return EXIT_FAILURE;
    }

    fprintf(file, "%s", json_string);
    fclose(file);

    cJSON_Delete(summary_json);
    free(json_string);

    return EXIT_SUCCESS;
}

void print_stats(tgn_stats* summary) {
    fprintf(stdout, "cnt_rx_pkts: %lu\n", summary->cnt_rx_pkts);
    fprintf(stdout, "cnt_tx_pkts: %lu\n", summary->cnt_tx_pkts);
    fprintf(stdout, "cnt_rx_bytes: %lu\n", summary->cnt_rx_bytes);
    fprintf(stdout, "cnt_tx_bytes: %lu\n", summary->cnt_tx_bytes);
    fprintf(stdout, "cnt_rx_pkts_qfull: %lu\n", summary->cnt_rx_pkts_qfull);
    fprintf(stdout, "cnt_rx_pkts_nombuf: %lu\n", summary->cnt_rx_pkts_nombuf);
    fprintf(stdout, "cnt_rx_pkts_err: %lu\n", summary->cnt_rx_pkts_err);
    fprintf(stdout, "cnt_tx_pkts_err: %lu\n", summary->cnt_tx_pkts_err);
}
