#include <stdint.h>
#include "utils.h"

#ifndef CI_H
#define CI_H

// ABI

#define __CX__         //(FIXME: compiler builtin)
#define CX_LEGACY 0

// TYPDEFS

typedef int32_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t cxu_id_t;         // system: CXU index

typedef int32_t cx_state_id_t;      // system: state index
typedef int32_t cx_vstate_id_t;      // system: state index
typedef int32_t cx_virt_t;       // context virtualization permissions

// typedef int32_t cx_index_t;        // hart: CX selector (value (No CX Table) or index
//                                  //       (when there is a CX Table))
typedef uint32_t cx_error_t;     //

typedef int32_t cxu_sctx_t;      // per state

typedef int32_t cx_select_t;      // per state

cx_select_t cx_open(cx_guid_t cx_guid, cx_virt_t cx_virt, cx_select_t ucx_select);
void cx_close(cx_select_t cx_select);

cx_error_t cx_error_read(void);
void cx_error_clear(void);
void cx_sel(cx_select_t);

#endif