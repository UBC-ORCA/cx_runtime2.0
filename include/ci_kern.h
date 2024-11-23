#include <linux/types.h>
#include "utils.h"

#ifndef CI_H
#define CI_H

// ABI

#define __CX__         //(FIXME: compiler builtin)
#define CX_LEGACY 0

// TYPDEFS

typedef s32 cx_guid_t;       // global: CX ID, a 128b GUID
typedef s32 cxu_id_t;        // system: CXU index

typedef s32 cx_vstate_id_t;  // virtual state id
typedef s32 cx_state_id_t;   // system: state index
typedef s32 cx_virt_t;       // context virtualization permissions

// typedef s32 cx_index_t;        // hart: CX selector (value (No CX Table) or index
//                                //       (when there is a CX Table))
typedef s32 cx_error_t;     //

typedef s32 cxu_sctx_t;      // per state
typedef s32 cx_sel_t;        // selector index

typedef s32 cx_select_t;

#endif
