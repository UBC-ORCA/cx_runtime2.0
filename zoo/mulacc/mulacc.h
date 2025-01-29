#include <stdint.h>
#include "../../include/ci.h"
#include "mulacc_common.h"

#ifndef MULACC_H
#define MULACC_H

__CX__ static inline int32_t mac( int32_t a, int32_t b ) {
    return CX_REG_HELPER(0, a, b);
};
__CX__ static inline int32_t reset( void ) {
    return CX_REG_HELPER(1, 0, 0);
};
__CX__ static inline int32_t do_nothing( void ) {
    return CX_REG_HELPER(2, 0, 0);
};

#endif