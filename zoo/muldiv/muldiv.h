#include <stdint.h>
#include "../../include/ci.h"
#include "muldiv_common.h"

#ifndef MULDIV_H
#define MULDIV_H

__CX__ static inline int32_t mul( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ static inline int32_t div_( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(1, a, b);
}

#endif