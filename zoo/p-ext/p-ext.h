#include <stdint.h>
#include "../../include/ci.h"
#include "p-ext_common.h"

#ifndef PEXT_H
#define PEXT_H

__CX__ static inline int32_t add16( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ static inline int32_t sra16( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(1, a, b);
}

__CX__ static inline int32_t smul16( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(2, a, b);
}

__CX__ static inline int32_t add8( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(3, a, b);
}

__CX__ static inline int32_t sra8( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(4, a, b);
}

__CX__ static inline int32_t smul8( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(5, a, b);
}

#endif