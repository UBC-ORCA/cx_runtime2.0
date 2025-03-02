#include "exports.h"

// structs populated by spike / qemu

int32_t (*cx_func_error[]) (int32_t, int32_t, cx_idx_t) = {0};

cx_func_stub_t cx_funcs[MAX_NUM_CXUS] = {
    cx_func_addsub,
    cx_func_muldiv,
    cx_func_mulacc, 
    cx_func_pext,
    cx_func_vector,
    cx_func_vector,
    cx_func_max,
    cx_func_nn_acc
    };

int32_t num_cfs[MAX_NUM_CXUS] = {
    CX_ADDSUB_NUM_FUNCS,
    CX_MULDIV_NUM_FUNCS, 
    CX_MULACC_NUM_FUNCS, 
    CX_PEXT_NUM_FUNCS,
    CX_VECTOR_NUM_FUNCS,
    CX_VECTOR_NUM_FUNCS,
    CX_MAX_NUM_FUNCS,
    CX_NN_ACC_NUM_FUNCS
    };

int32_t num_states[MAX_STATE_SIZE] = {
    CX_ADDSUB_NUM_STATES,
    CX_MULDIV_NUM_STATES,
    CX_MULACC_NUM_STATES,
    CX_PEXT_NUM_STATES,
    CX_VECTOR_NUM_STATES,
    CX_VECTOR_NUM_STATES,
    CX_MAX_NUM_STATES,
    CX_NN_ACC_NUM_STATES
};

// Fill unused functions in their arrays error
void cx_init_funcs() {
    init_cx_func_mulacc();
    init_cx_func_vector();
    init_cx_func_nn_acc();

    for (int i = NUM_CXUS; i < MAX_NUM_CXUS; i++) {
        cx_funcs[i] = cx_func_error;
        num_cfs[i] = 0;
        num_states[i] = 0;
    }
}