// This code is here because the baremetal version happened at some arbitrary point
// between different versions of various CX related registers, instructions, etc.
// Eventually, when we have a board implementation that supports the spec'ed 
// cx_state_context_status CSR, we should be fine to use that. 

#ifndef UTILS_M_H
#define UTILS_M_H

#define CX_SELECTOR_USER 0x800 
#define CX_INDEX         0xBC0
#define CX_STATUS        0x801
#define MCX_SELECTOR     0xBC0

#define NUM_CXUS 4

typedef unsigned int uint;
// #define uint unsigned

#define CUSTOM0 0xb
#define CUSTOM1 0x2b
#define CUSTOM2 0x5b

#define CX_REG_TYPE  CUSTOM0
#define CX_IMM_TYPE  CUSTOM1
#define CX_FLEX_TYPE CUSTOM2

#define CX_INVALID_SELECTOR 0x80000000

#define CX_HW_INIT 0
#define CX_OS_INIT 1

/* CX_INDEX */
#define CX_CXU_ID_START_INDEX 0
#define CX_CXU_ID_BITS 4

#define CX_VIRT_STATE_START_INDEX 8
#define CX_VIRT_STATE_BITS 8

#define CX_STATE_ID_START_INDEX 16
#define CX_STATE_ID_BITS 4

#define CX_VERSION_START_INDEX 28
#define CX_VERSION_BITS 3

/* CX_STATUS */
#define CX_IS_BITS 1
#define CX_IS_START_INDEX 0

#define CX_IF_BITS 1
#define CX_IF_START_INDEX 1

/* CX_STATE_CONTEXT_STATUS */
#define CX_STATUS_START_INDEX 0
#define CX_STATUS_BITS 2

#define CX_STATE_SIZE_START_INDEX 3
#define CX_STATE_SIZE_BITS 10

#define CX_NUM_QUEUES_START_INDEX 13
#define CX_NUM_QUEUES_BITS 4

#define CX_ERROR_START_INDEX 24
#define CX_ERROR_BITS 8

#define CX_INITIALIZER_START_INDEX 2
#define CX_INITIALIZER_BITS 1

#define GET_BITS(cx_sel, start_bit, n) \
    ((cx_sel >> start_bit) & (((1 << n) - 1) ))

/* CX_SELECTOR_USER MACROS */

#define CX_GET_CXU_ID(cx_sel) \
    GET_BITS(cx_sel, CX_CXU_ID_START_INDEX, CX_CXU_ID_BITS)

#define CX_GET_STATE_ID(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_ID_START_INDEX, CX_STATE_ID_BITS)

#define CX_GET_VIRT_STATE_ID(cx_sel) \
    GET_BITS(cx_sel, CX_VIRT_STATE_START_INDEX, CX_VIRT_STATE_BITS)

#define CX_GET_IV(cx_sel) \
    GET_BITS(cx_sel, CX_IV_START_INDEX, CX_IV_BITS)

// ========= cx state context status helpers ===========

#define GET_CX_STATUS(cx_sel) \
    GET_BITS(cx_sel, CX_STATUS_START_INDEX, CX_STATUS_BITS)

#define GET_CX_INITIALIZER(cx_sel) \
    GET_BITS(cx_sel, CX_INITIALIZER_START_INDEX, CX_INITIALIZER_BITS)

#define GET_CX_STATE_SIZE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_SIZE_START_INDEX, CX_STATE_SIZE_BITS)

#define GET_CX_NUM_QUEUES(cx_sel) \
    GET_BITS(cx_sel, CX_NUM_QUEUES_START_INDEX, CX_NUM_QUEUES_BITS)

#define GET_CX_ERROR(cx_sel) \
    GET_BITS(cx_sel, CX_ERROR_START_INDEX, CX_ERROR_BITS)

#define MAX_NUM_STATES (1 << CX_STATE_ID_BITS)
#define MAX_NUM_CXUS   (1 << CX_CXU_ID_BITS)
#define MAX_CF_IDS    (1 << 10)
#define MAX_NUM_VSTATES (1 << CX_VIRT_STATE_BITS)
#define VSTATE_WORDS (MAX_NUM_VSTATES / 32)

#define MAX_STATE_SIZE 1024 // number of words in a state
#define CX_LEGACY 0

#define UNASSIGNED_VIRT -1

enum CX_CS {
    CX_OFF, 
    CX_INITIAL, 
    CX_CLEAN,
    CX_DIRTY
};

typedef enum {
	CX_NO_VIRT,
    CX_INTRA_VIRT,
	CX_INTER_VIRT,
    CX_FULL_VIRT
} CX_VIRT_T;

typedef union {
    struct {
        uint cxu_id     : CX_CXU_ID_BITS;
        uint reserved0  : 4;
        uint v_state_id : CX_VIRT_STATE_BITS;
        uint state_id   : CX_STATE_ID_BITS;
        uint reserved1  : 9;
        uint version    : CX_VERSION_BITS;
    } sel;
        uint idx;
 } cx_idx_t;

typedef union {
    struct {
        uint IS        : CX_IS_BITS;
        uint IF        : CX_IF_BITS;
        uint reserved0 : 29;
    } sel;
    int idx;
} cx_status_t;

typedef union {
    struct {
       uint cs          : CX_STATUS_BITS;
       uint initializer : CX_INITIALIZER_BITS;
       uint state_size  : CX_STATE_SIZE_BITS;
       uint num_queues  : CX_NUM_QUEUES_BITS;
       uint reserved0   : 7;
       uint error       : CX_ERROR_BITS;
    } sel;
     uint idx;
} cx_stctxs_t;

#define CX_REG_HELPER(cf_id, rs1, rs2)      ({           \
    register int __v;                                    \
    asm volatile("      cx_reg " #cf_id ",%0,%1,%2;\n\t" \
                 : "=r" (__v)                            \
                 : "r" (rs1), "r" (rs2)                  \
                 : "memory"                              \
    );                                                   \
	__v;							                     \
})

#define __ASM_STR(x)	#x
#define cx_csr_read(csr)				                \
({								                        \
	register unsigned int __v;				            \
	__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
			      : "=r" (__v) :			            \
			      : "memory");			                \
	__v;							                    \
})

#define cx_csr_write(csr, val)					        \
({								                        \
	unsigned int __v = (unsigned int)(val);		        \
	__asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			            \
			      : "memory");			                \
})

#define CX_READ_STATUS()                CX_REG_HELPER(1023, 0, 0)
#define CX_WRITE_STATUS(status)         CX_REG_HELPER(1022, status, 0)
#define CX_READ_STATE(index)            CX_REG_HELPER(1021, index, 0)
#define CX_WRITE_STATE(index, value)    CX_REG_HELPER(1020, index, value)

#endif