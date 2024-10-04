#ifndef EXPORTS_H
#define EXPORTS_H

#include <stdint.h>

#include "../include/utils.h"
#include "vector/vector_func.h"


#define CX_ERROR_NUM_FUNCS 0

typedef int32_t (*(*cx_func_stub_t)) (int32_t, int32_t, int32_t);

extern int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t);

extern cx_func_stub_t cx_funcs[MAX_NUM_CXUS];

extern int32_t num_cfs[MAX_NUM_CXUS];

extern int32_t num_states[MAX_NUM_CXUS];

void cx_init_funcs( void );

#endif // EXPORTS_H