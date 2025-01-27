#include <stdint.h>

#include "max.h"

__CX__ int32_t max( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}