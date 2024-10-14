#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/utils.h"

#include "../zoo/vector/vector_common.h"

#define CX_AVAIL_STATE 1
#define CX_UNAVAIL_STATE 0

typedef struct cx_state_data_t {
    int status;
    int v_id;
    uint *data;
    struct cx_state_data_t *next;
} cx_state_data_t;

typedef struct cx_state_info_t {
    CX_VIRT_T virt;
    // when the counter is 0, we can set the CX_VIRT_T. Until it becomes
    // 0'ed again, we must respect that all newly opened virtual
    // contexts are of the same virt type, or else the cx_open will fail.
    cx_state_data_t *vstate;
    uint vstate_id[4]; // 128 virtual IDs per state context.
    int vcounter;
    int prev_used_vid; // To know if context save + restore is needed
} cx_state_info_t;

typedef struct {
  // static
  int cx_guid;
  int num_states;

  // dynamic
  int *avail_state_ids;
  cx_state_info_t *state_info;
} cx_entry_m_t;

typedef cx_entry_m_t cx_map_t;
cx_map_t cx_map[NUM_CXUS];

static void list_add(cx_state_data_t *data, cxu_id_t cxu_id, cx_state_id_t state_id) {
    cx_state_data_t *curr = cx_map[cxu_id].state_info[state_id].vstate;

    if (curr == NULL) {
        cx_map[cxu_id].state_info[state_id].vstate = data;
        return;
    }

    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = data;
}

static void list_del(cx_select_t cx_sel) {
    cx_idx_t cx_sel_ = { .idx = cx_sel };
    cx_state_data_t *curr = cx_map[cx_sel_.sel.cxu_id].state_info[cx_sel_.sel.state_id].vstate;

    if (curr == NULL) {
        return;
    }

    if (curr->v_id == cx_sel_.sel.v_state_id) {
        cx_map[cx_sel_.sel.cxu_id].state_info[cx_sel_.sel.state_id].vstate = curr->next;
        free(curr);
        return;
    }

    while (curr->next != NULL && curr->next->v_id != cx_sel_.sel.v_state_id) {
        curr = curr->next;
    }

    cx_state_data_t *tmp = curr;


}

static inline cx_select_t gen_cx_sel(cxu_id_t cxu_id, cx_state_id_t state_id, 
                                     cx_vstate_id_t vstate_id) 
{
    cx_idx_t cx_sel = {.sel = {   .cxu_id = cxu_id, 
                                  .state_id = state_id,
                                  .v_state_id = vstate_id,
                                  .version = 1}};
    return cx_sel.idx;
}


// breadth first search across all cxu's for the lowest free state id
static cx_select_t get_state(cxu_id_t cxu_id) {

    cx_guid_t cx_guid = cx_map[cxu_id].cx_guid;

    cxu_id_t new_cxu_id = cxu_id;
    cx_state_id_t state_id = MAX_NUM_STATES;

    int lowest_use_state_count = 0x7FFFFFFF;
    int lowest_use_vstate = MAX_NUM_STATES;
    for (int i = cxu_id; i < NUM_CXUS; i++) {
        if (cx_map[i].cx_guid != cx_guid) {
            continue;
        }
        for (int j = 0; j < cx_map[i].num_states; j++) {
            if (cx_map[i].avail_state_ids[j] == 1) {
                if (j < state_id) {
                    new_cxu_id = i;
                    state_id = j;
                    lowest_use_state_count = 0;
                }
                break;
            } else { // Finding lowest virtualized state count
                if (lowest_use_state_count != 0 &&
                    cx_map[i].state_info[j].vcounter < lowest_use_state_count) {
                    new_cxu_id = i;
                    lowest_use_vstate = j;
                    lowest_use_state_count = cx_map[i].state_info[j].vcounter;
                }
            }
        }
    }
    // Only room to virtualize 128 times
    if (lowest_use_state_count >= 128) {
        return -1;
    }

    if (state_id < MAX_NUM_STATES) {
        cx_map[new_cxu_id].avail_state_ids[state_id] = 0;
        cx_select_t cx_sel = gen_cx_sel(new_cxu_id, state_id, 0);
        return cx_sel;
    }

    cx_map[new_cxu_id].avail_state_ids[lowest_use_vstate] = 0;
    return gen_cx_sel(new_cxu_id, lowest_use_vstate, 0);
}

static int get_free_vstate(cxu_id_t cxu_id, cx_state_id_t state_id) {
    for (int i = 0; i < 4; i++) { // 4 * 32 = 128, number of virt_id's per physical state context
        if (cx_map[cxu_id].state_info[state_id].vstate_id[i] != 0xFFFFFFFF) {
            cx_vstate_id_t vstate = cx_map[cxu_id].state_info[state_id].vstate_id[i];
            for (int j = 0; j < 32; j++) { // iterating over bits
                if (GET_BITS(vstate, j, 1) == 0) {
                    vstate |= (1 << j);
                    cx_map[cxu_id].state_info[state_id].vstate_id[i] = vstate;
                    cx_map[cxu_id].state_info[state_id].vcounter++;
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;
}

static void free_vstate(cxu_id_t cxu_id, cx_state_id_t state_id, cx_vstate_id_t vstate_id) {
    cx_map[cxu_id].state_info[state_id].vstate_id[vstate_id / 4] |= (1 << (vstate_id % 32));
}

static bool is_state_unused(cxu_id_t cxu_id, cx_state_id_t state_id) {
    for (int i = 0; i < 4; i++) {
        if (cx_map[cxu_id].state_info[state_id].vstate_id[i] != 0) {
            return false;
        }
    }
    return true;
}

static int is_valid_state_id(cxu_id_t cxu_id, cx_state_id_t state_id) 
{
    if (state_id < 0) {
        return false; // No available states for cx_guid 
    } else if (state_id > cx_map[cxu_id].num_states - 1) {
        return false;
    }
    return true;
}

// populates the cx_map
void cx_init() {
    // zero the cx_index
    cx_csr_write(CX_INDEX, 0);
    
    // 0 initialize the cx_status csr
    // cx_csr_write(CX_STATUS, 0);

    cx_map[0].cx_guid = CX_GUID_VECTOR;
    cx_map[0].num_states = CX_VECTOR_NUM_STATES;

    cx_map[1].cx_guid = CX_GUID_VECTOR;
    cx_map[1].num_states = CX_VECTOR_NUM_STATES;

    for (int i = 0; i < NUM_CXUS; i++) {
        cx_map[i].avail_state_ids = malloc(cx_map[i].num_states * sizeof(int));
        cx_map[i].state_info = malloc(cx_map[i].num_states * sizeof(cx_state_info_t));
        for (int j = 0; j < cx_map[i].num_states; j++) {
            cx_map[i].state_info[j].vstate = NULL;
            cx_map[i].avail_state_ids[j] = 1;
            cx_map[i].state_info[j].virt = -1;
            cx_map[i].state_info[j].vcounter = 0;
            cx_map[i].state_info[j].prev_used_vid = -1;
            for (int k = 0; k < 4; k++) {
                cx_map[i].state_info[j].vstate_id[k] = 0;
            }
        }
    }
}

void context_save(cx_state_data_t *data) {
    uint status = CX_READ_STATUS();
    int size = GET_CX_STATE_SIZE(status);
    for (int i = 0; i < size; i++) {
        data->data[i] = CX_READ_STATE(i);
    }

    cx_stctxs_t status_word = { .idx = status };
    status_word.sel.cs = CX_CLEAN;
    data->status = status_word.idx;
}

void context_restore(cx_state_data_t *data) {
    int size = GET_CX_STATE_SIZE(data->status);
    for (int i = 0; i < size; i++) {
        CX_WRITE_STATE(i, data->data[i]);
    }

    CX_WRITE_STATUS(data->status);
}

void cx_sel(cx_select_t cx_sel) {
    cx_idx_t new_sel =  { .idx = cx_sel };
    int prev_used_vid = cx_map[new_sel.sel.cxu_id].state_info[new_sel.sel.state_id].prev_used_vid;

    cx_csr_write(CX_INDEX, cx_sel);

    if (cx_sel == CX_LEGACY) {
        return;
    }

    if (prev_used_vid == -1 ||
        new_sel.sel.v_state_id == prev_used_vid) {
        cx_map[new_sel.sel.cxu_id].state_info[new_sel.sel.state_id].prev_used_vid = new_sel.sel.v_state_id;
        return;
    }

    // context switching needed
    cx_state_data_t *new_state_data = NULL, *prev_state_data = NULL, 
                    *tmp_state_data = cx_map[new_sel.sel.cxu_id].state_info[new_sel.sel.state_id].vstate;
    while (tmp_state_data != NULL) {
        if (tmp_state_data->v_id == prev_used_vid) {
            prev_state_data = tmp_state_data;
        }

        if (tmp_state_data->v_id == new_sel.sel.v_state_id) {
            new_state_data = tmp_state_data;
        }

        if (new_state_data && prev_state_data) {
            break;
        }
        tmp_state_data = tmp_state_data->next;
    }

    context_save(prev_state_data);
    context_restore(new_state_data);

    cx_map[new_sel.sel.cxu_id].state_info[new_sel.sel.state_id].prev_used_vid = new_sel.sel.v_state_id;
}

static void initialize_state() {

    uint status = CX_READ_STATUS();
    uint sw_init = GET_CX_INITIALIZER(status);
    // CX_WRITE_STATUS(CX_INITIAL);

    // hw required to set to dirty after init, while sw does it explicitly
    if (sw_init) {
        uint size = GET_CX_STATE_SIZE(status);
        for (int i = 0; i < size; i++) {
            CX_WRITE_STATE(i, 0);
        }
        // CX_WRITE_STATUS(CX_DIRTY);
    }
}

static cx_select_t alloc_sel(cxu_id_t cxu_id) {
    cx_select_t partial_sel = get_state(cxu_id);
    if (partial_sel == -1) {
        return -1;
    }

    cxu_id_t new_cxu_id = CX_GET_CXU_ID(partial_sel);
    cx_state_id_t state_id = CX_GET_STATE_ID(partial_sel);

    if (!is_valid_state_id(new_cxu_id, state_id)) {
        return -1;
    }

    cx_vstate_id_t vstate_id = get_free_vstate(new_cxu_id, state_id);

    if (vstate_id == -1) {
        return -1;
    }


    cx_select_t new_cx_sel = gen_cx_sel(new_cxu_id, state_id, vstate_id);
    cx_state_data_t *state = malloc(sizeof(cx_state_data_t));
    if (!state) {
        return -1;
    }
    
    state->data = malloc(sizeof(uint) * MAX_STATE_SIZE);
    if (!state->data) {
        return -1;
    }

    state->v_id = vstate_id;
    
    list_add(state, new_cxu_id, state_id);
    return new_cx_sel;
}

static int initialized = 0;

int32_t cx_open(cx_guid_t cx_guid, cx_virt_t cx_virt, cx_select_t user_cx_sel) {
    if (!initialized) {
        cx_init();
        initialized = 1;
    }
    cxu_id_t cxu_id = -1;
    for (int j = 0; j < NUM_CXUS; j++) {
        if (cx_map[j].cx_guid == cx_guid) {
            cxu_id = j;
            break;
        }
    }
    
    if (cxu_id == -1) {
       return -1;
    }

    if (cx_map[cxu_id].num_states == 0) {
        return gen_cx_sel(cxu_id, 0, 0);
    } else {

        // Note: After caling alloc_sel, cxu_id could be changed.
        cx_select_t new_cx_sel = alloc_sel(cxu_id);
        if (new_cx_sel == -1) {
            return -1;
        }

        cx_select_t prev_sel = cx_csr_read(CX_INDEX);
        cx_sel(new_cx_sel);

        initialize_state();

        cx_sel(prev_sel);

        return new_cx_sel;
    }
}


void cx_close(cx_select_t cx_sel)
{
  cxu_id_t cxu_id = CX_GET_CXU_ID(cx_sel);
  
  if (cxu_id >= NUM_CXUS) {
    return;
  }
  // Stateless cx's
  if (cx_map[cxu_id].num_states == 0) {
    return;
  } else { // Stateful
    cx_state_id_t state_id = CX_GET_STATE_ID(cx_sel);
    cx_vstate_id_t vstate_id = CX_GET_VIRT_STATE_ID(cx_sel);
    cx_map[cxu_id].avail_state_ids[state_id] = 1;

    cx_state_data_t* state_data;

    list_del(cx_sel);

    free_vstate(cxu_id, state_id, vstate_id);

    if (is_state_unused(cxu_id, state_id)) {
        cx_map[cxu_id].state_info[state_id].virt = -1;
        cx_map[cxu_id].state_info[state_id].prev_used_vid = -1;
    }

  }
}

cx_error_t cx_error_read() {
  return cx_csr_read(CX_STATUS);
}

void cx_error_clear() {
  cx_csr_write(CX_STATUS, 0);
}

