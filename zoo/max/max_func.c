#include <stdint.h>
#include "../../include/utils.h"

#include "max_func.h"

static inline int32_t max_func(int32_t a, int32_t b, cx_idx_t sys_sel)
{
    if (a > b) {
        return a;
    }
    return b;
}

int32_t (*cx_func_max[]) (int32_t, int32_t, cx_idx_t) = {
    max_func
};