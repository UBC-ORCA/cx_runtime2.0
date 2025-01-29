#ifndef EXPORTS_H
#define EXPORTS_H

#include <stdint.h>

#include "../include/utils.h"
#include "addsub/addsub_func.h"
#include "muldiv/muldiv_func.h"
#include "mulacc/mulacc_func.h"
#include "p-ext/p-ext_func.h"
#include "vector/vector_func.h"
#include "max/max_func.h"
#include "nn_acc/nn_acc_func.h"


#define CX_ERROR_NUM_FUNCS 0

typedef int32_t (*(*cx_func_stub_t)) (int32_t, int32_t, cx_idx_t);

extern int32_t (*cx_func_error[]) (int32_t, int32_t, cx_idx_t);

extern cx_func_stub_t cx_funcs[MAX_NUM_CXUS];

extern int32_t num_cfs[MAX_NUM_CXUS];

extern int32_t num_states[MAX_STATE_SIZE];

void cx_init_funcs( void );

#endif // EXPORTS_H