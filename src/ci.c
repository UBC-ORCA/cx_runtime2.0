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
    uint32_t status;
    uint *data;
    struct list_head v_contexts;
} cx_virt_data_t;

typedef struct state_info_t {
    struct list_head cx_virt_data;
} state_info_t;

typedef struct cxu_info_t {
    cx_guid_t cx_guid[1];
    state_info_t state_info[MAX_NUM_STATES];
    int state_type;
} cxu_info_t;

// virtual selector table
int32_t vst[NUM_CXUS][MAX_NUM_STATES];

enum {
    CX_UNDEFINED,
    CX_STATELESS,
    CX_STATEFUL
};

static cxu_info_t cxu[NUM_CXUS];

// accessed the same way that we access the enable bits
// e.g., indices 0-15 are for CXU_0, states 0-15;
//       indices 16-31 are for CXU_1, states 0-15, etc.
// The OS will update this table with data each time that 
// we do a context switch
static int8_t cxu_permission_table[MAX_NUM_CXUS * MAX_NUM_STATES];

static cx_select_t gen_cx_sel(cxu_id_t cxu_id, cx_state_id_t state_id,
                                  cx_vstate_id_t vstate_id)
{
    cx_idx_t cx_sel = {.sel = {   .cxu_id = cxu_id,
                                  .state_id = state_id,
                                  .v_state_id = vstate_id,
                                  .version = 1,
                                  .iv = 0}};
    return cx_sel.idx;
}

static void init_virt_data(cx_virt_data_t *ptr, int32_t vaddr) {
    ptr->data = malloc(sizeof(int) * MAX_STATE_SIZE);
    if (!ptr->data) {
        printf("Error allocating memory for state context data\n");
    }
    ptr->status = CX_READ_STATUS();
    for (int i = 0; i < GET_CX_STATE_SIZE(ptr->status); i++) {
        ptr->data[i] = CX_READ_STATE(i);
    }
    ptr->virt_addr = vaddr;
}

static void del_virt_data(cx_virt_data_t *ptr) {
    ptr->status = 0;
    ptr->virt_addr = -1;
    free(ptr->data);
    ptr->data = NULL;
}

static cx_virt_data_t *get_virtual_state(state_info_t *head, int32_t virt_addr) 
{
    cx_virt_data_t *p;
    list_for_each_entry(p, &head->cx_virt_data, v_contexts) {
        if (p->virt_addr == virt_addr) {
            return p;
        }
    };
    return NULL;
}

static void del_ucxt(cx_select_t sel) {
    cxu_id_t cxu_id = CX_GET_CXU_ID(sel);
    cx_state_id_t state_id = CX_GET_STATE_ID(sel);
    cx_vstate_id_t vstate_id = CX_GET_VIRT_STATE_ID(sel);
    cx_virt_data_t *p = get_virtual_state(&cxu[cxu_id].state_info[state_id], vstate_id);
    if (!p) {
        printf("couldn't find state to free\n");
        exit(1);
    }
    list_del(&p->v_contexts);
    del_virt_data(p);
}

static void ucxt_save(cxu_id_t cxu_id, cx_state_id_t state_id) {
    cx_vstate_id_t vstate = vst[cxu_id][state_id];
    cx_virt_data_t *p = get_virtual_state(&cxu[cxu_id].state_info[state_id], vstate);
    if (!p) {
        printf("couldn't find state context to save; cxu: %d, state: %d, vstate: %d\n", cxu_id, state_id, vstate);
        exit(1);
    }

    // This is where the program should trap to the OS, if the blob in the physical state context
    // is owned by a selector in another process
    // The OS will save + restore, but user mode must also do a save + restore to get the 
    // correct blob in the state context (as the new blob is stored in user-land)
    cx_csr_write(CX_SELECTOR_USER, gen_cx_sel(cxu_id, state_id, vstate));
    p->status = CX_READ_STATUS();
    cx_stctxs_t status = {.idx = p->status};
    for (int i = 0; i < status.sel.state_size; i++) {
        p->data[i] = CX_READ_STATE(i);
    }
}

static void ucxt_restore(cx_idx_t sel) {
    cx_virt_data_t *p = get_virtual_state(&cxu[sel.sel.cxu_id].state_info[sel.sel.state_id], sel.sel.v_state_id);
    if (!p) {
        printf("couldn't find the virtual state context\n");
    }
    cx_stctxs_t status = {.idx = p->status};
    
    if (status.sel.state_size > MAX_STATE_SIZE) {
        printf("Issue getting max state size (too large)");
        exit(1);
    }
    for (int i = 0; i < status.sel.state_size; i++) {
        CX_WRITE_STATE(i, p->data[i]);
    }

    status.sel.cs = CX_CLEAN;
    CX_WRITE_STATUS(status);
}

static void init_ucxt(cx_select_t sel) {
    cx_virt_data_t *p = malloc(sizeof(cx_virt_data_t));
    if (!p) {
        printf("Error allocating memory for state context\n");
    }
    cxu_id_t cxu_id = CX_GET_CXU_ID(sel);
    cx_state_id_t state_id = CX_GET_STATE_ID(sel);
    cx_vstate_id_t vstate_id = CX_GET_VIRT_STATE_ID(sel);

    cx_select_t prev_sel = cx_csr_read(CX_SELECTOR_USER);

    // can't use cx_sel() here, because we haven't initialized the new context
    if (vst[cxu_id][state_id] >= 0) {
        cx_select_t vst_sel = gen_cx_sel(cxu_id, state_id, vst[cxu_id][state_id]);
        cx_csr_write(CX_SELECTOR_USER, vst_sel);
        ucxt_save(cxu_id, state_id);
    }

    cx_csr_write(CX_SELECTOR_USER, sel);
    init_virt_data(p, vstate_id);

    if (vst[cxu_id][state_id] >= 0) {
        cx_idx_t prev_sel_idx = {.idx = prev_sel};
        ucxt_restore(prev_sel_idx);
    }

    cx_csr_write(CX_SELECTOR_USER, prev_sel);
    list_add(&p->v_contexts, &cxu[cxu_id].state_info[state_id].cx_virt_data);
}

static void ucxt_switch(cx_idx_t new_sel) {
    ucxt_save(new_sel.sel.cxu_id, new_sel.sel.state_id);
    ucxt_restore(new_sel);
}

void inline cx_sel(cx_select_t sel) {
    cx_idx_t _sel = {.idx = sel};
    if (_sel.idx != CX_LEGACY &&
        _sel.idx != CX_INVALID_SELECTOR &&
        cxu[_sel.sel.cxu_id].state_type == CX_STATEFUL &&
        vst[_sel.sel.cxu_id][_sel.sel.state_id] >= 0 &&
        vst[_sel.sel.cxu_id][_sel.sel.state_id] != _sel.sel.v_state_id) {
        ucxt_switch(_sel);
    }
    if (_sel.idx != CX_LEGACY &&
        _sel.idx != CX_INVALID_SELECTOR &&
        cxu[_sel.sel.cxu_id].state_type == CX_STATEFUL) {
        vst[_sel.sel.cxu_id][_sel.sel.state_id] = _sel.sel.v_state_id;
    }
    cx_csr_write(CX_SELECTOR_USER, sel);
}

static void cx_init() {
    cxu[0].state_type = CX_STATELESS;
    cxu[1].state_type = CX_STATELESS;
    cxu[2].state_type = CX_STATEFUL; // mulacc
    cxu[3].state_type = CX_STATELESS;

    for (int i = 0; i < NUM_CXUS; i++) {
        for (int j = 0; j < MAX_NUM_STATES; j++) {
            INIT_LIST_HEAD(&cxu[i].state_info[j].cx_virt_data);
            vst[i][j] = -1;
        }
    }
}

bool initialized = false;
cx_select_t cx_open(cx_guid_t cx_guid, cx_virt_t cx_virt, cx_select_t ucx_select) {
    if (!initialized) {
        cx_init();
        initialized = true;
    }
    // register long sel asm("a0");
    register long a0 asm("a0") = cx_guid;
    register long a1 asm("a1") = cx_virt;
    register long a2 asm("a2") = ucx_select;
    register long syscall_id asm("a7") = 457; // cx_open
    asm volatile ("ecall"
        : "+r"(a0)
        : "r"(a0), "r"(a1), "r"(a2), "r"(syscall_id)
        :
    );
    int sel = a0;
    if (sel < 0) {
        return -1;
    }
    cxu_id_t cxu_id = CX_GET_CXU_ID(sel);
    if (cxu[cxu_id].state_type == CX_STATELESS) {
        return sel;
    }
    if (cxu[cxu_id].state_type == CX_UNDEFINED) {
        printf("undefined state type\n");
        exit(1);
    }
    init_ucxt(sel);
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
    cxu_id_t cxu_id = CX_GET_CXU_ID(sel);
    cx_state_id_t state_id = CX_GET_STATE_ID(sel);
    if (cxu[cxu_id].state_type == CX_STATELESS) {
        return;
    }
    vst[cxu_id][state_id] = -1;
    del_ucxt(sel);
//   cx_csr_write(CX_SELECTOR_USER, CX_LEGACY);
}

void cx_error_clear() {
    return;
}

cx_error_t cx_error_read() {
    return 0;
}