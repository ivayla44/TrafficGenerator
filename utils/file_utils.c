#include "file_utils.h"

bool set_max_fds(uint32_t cnt) {
    struct rlimit cur_limits = {};

    if(getrlimit(RLIMIT_NOFILE, &cur_limits) != 0) {
        fprintf(stderr, "Error getting sys limits - err: %d, value: %s\n", errno, strerror(errno));
        return false;
    }

    struct rlimit new_limits = cur_limits;
    new_limits.rlim_cur = cnt;
    if(cnt > new_limits.rlim_max) new_limits.rlim_max = cnt;

    if(setrlimit(RLIMIT_NOFILE, &new_limits) != 0) {
        fprintf(stderr, "Error setting sys limits- err: %d, value: %s\n", errno, strerror(errno));
        return false;
    }

    return true;
}
