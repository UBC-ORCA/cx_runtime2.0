#include "exports.h"

int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t) = {0};

cx_func_stub_t cx_funcs[MAX_NUM_CXUS] = {
    cx_func_vector,
    cx_func_vector
    };

int32_t num_cfs[MAX_NUM_CXUS] = {
    CX_VECTOR_NUM_FUNCS,
    CX_VECTOR_NUM_FUNCS
    };

int32_t num_states[MAX_NUM_CXUS] = {
    CX_VECTOR_NUM_STATES,
    CX_VECTOR_NUM_STATES
};

// Fill unused functions in their arrays error
void cx_init_funcs() {
    init_cx_func_vector();

    for (int i = NUM_CXUS; i < MAX_NUM_CXUS; i++) {
        cx_funcs[i] = cx_func_error;
        num_cfs[i] = 0;
        num_states[i] = 0;
    }
}