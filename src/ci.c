#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/list.h"

#include "../zoo/muldiv/muldiv_common.h"
#include "../zoo/addsub/addsub_common.h"
#include "../zoo/mulacc/mulacc_common.h"
#include "../zoo/p-ext/p-ext_common.h"

typedef struct cx_virt_data_t {
    int32_t virt_addr;
    int32_t size;
    uint *data;
    struct list_head v_contexts;
} cx_virt_data_t;

typedef struct state_info_t {
    cx_virt_t virt;
    struct list_head cx_virt_data;
} state_info_t;

typedef struct cxu_info_t {
    cx_guid_t cx_guid[1];
    state_info_t state_info[MAX_NUM_STATES];
    int32_t num_states;
} cxu_info_t;

// virtual selector table
uint vst[NUM_CXUS][MAX_NUM_STATES];

static cxu_info_t cxu[NUM_CXUS];

// accessed the same way that we access the enable bits
// e.g., indices 0-15 are for CXU_0, states 0-15;
//       indices 16-31 are for CXU_1, states 0-15, etc.
// The OS will update this table with data each time that 
// we do a context switch
static int8_t cxu_permission_table[MAX_NUM_CXUS * MAX_NUM_STATES];

static inline bool cx_enable_read(cx_idx_t sel) {

    cxu_id_t cxu_csr = sel.sel.cxu_id >> 2;
    uint cx_enable_csr = 0;

    if (cxu_csr > 3) {
        printf("CSR out of range\n");
        exit(1);
    } else {
        // TODO: Read the correct enable csr
        cx_enable_csr = cx_csr_read(MCX_ENABLE0);
    }
    
    bool enable = GET_BITS(cx_enable_csr, sel.sel.state_id, 1);
    return enable;
}

void init_virt_data(cx_virt_data_t *ptr) {
    ptr->data = NULL;
    ptr->size = 0;
    ptr->virt_addr = -1;
}

cx_virt_data_t *get_virtual_state(state_info_t *head, int32_t virt_addr) 
{
    cx_virt_data_t *p;
    list_for_each_entry(p, &head->cx_virt_data, v_contexts) {
        if (p->virt_addr == virt_addr) {
            return p;
        }
    };
    return NULL;
}


// void cx_sel(cx_select_t sel) {
//     // inline this
//     cx_idx_t prev_cx_idx = {.idx = cx_csr_read(CX_SELECTOR_USER)};
//     cx_csr_write(CX_SELECTOR_USER, sel);

//     // separate this
//     cx_idx_t new_cx_idx = {.idx = sel};
//     if (vst[new_cx_idx])
// }


int32_t cx_init()
{
    cxu[0].cx_guid[0] = CX_GUID_ADDSUB;
    cxu[1].cx_guid[0] = CX_GUID_MULDIV;
    cxu[2].cx_guid[0] = CX_GUID_MULACC;
    cxu[3].cx_guid[0] = CX_GUID_PEXT;

    cxu[0].num_states = CX_ADDSUB_NUM_STATES;
    cxu[1].num_states = CX_MULDIV_NUM_STATES;
    cxu[2].num_states = CX_MULACC_NUM_STATES;
    cxu[3].num_states = CX_PEXT_NUM_STATES;

    for (int i = 0; i < NUM_CXUS; i++) {
        for (int j = 0; j < cxu[i].num_states; j++) {
            INIT_LIST_HEAD(&cxu[i].state_info[j].cx_virt_data);
            cxu[i].state_info[j].virt = -1;
        }
    }
}

cx_select_t cx_open(cx_guid_t cx_guid, cx_virt_t cx_virt, cx_select_t ucx_select) {
    register long sel asm("a0");
    register long a0 asm("a0") = cx_guid;
    register long a1 asm("a1") = cx_virt;
    register long a2 asm("a2") = ucx_select;
    register long syscall_id asm("a7") = 457; // cx_open
    asm volatile ("ecall  # 0=%0   1=%1  2=%2  3=%3 4=%4"
        : "=r"(sel)
        : "r"(a0), "r"(a1), "r"(a2), "r"(syscall_id)
        :
    );
    return sel;
}

void cx_close(cx_select_t sel) {
  int cx_close_error = -1;
  asm volatile (
    "li a7, 458;        \n\t"  // syscall 458, cx_close
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t"
    :  "=r" (cx_close_error)
    :  "r"  (sel)  
    : 
  );
}

void cx_sel(cx_select_t sel) {
    return cx_csr_write(CX_SELECTOR_USER, sel);
}

void cx_error_clear() {
    return;
}

cx_error_t cx_error_read() {
    return 0;
}