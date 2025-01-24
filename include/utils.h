#ifndef UTILS_H
#define UTILS_H

#define QEMU
#ifdef QEMU
    #define MCX_ENABLE0       0x018 // CXU 0, STATE_ID 0-16; CXU 1, STATE_ID 0-16
    #define MCX_ENABLE1       0x019 // CXU 2, STATE_ID 0-16; CXU 3, STATE_ID 0-16
    #define MCX_ENABLE2       0x01A // CXU 4, STATE_ID 0-16; CXU 5, STATE_ID 0-16
    #define MCX_ENABLE3       0x01B // CXU 6, STATE_ID 0-16; CXU 7, STATE_ID 0-16
    #define MCX_ENABLE4       0x01C // CXU 8, STATE_ID 0-16; CXU 9, STATE_ID 0-16
    #define MCX_ENABLE5       0x01D // CXU 10, STATE_ID 0-16; CXU 11, STATE_ID 0-16
    #define MCX_ENABLE6       0x01E // CXU 12, STATE_ID 0-16; CXU 13, STATE_ID 0-16
    #define MCX_ENABLE7       0x01F // CXU 14, STATE_ID 0-16; CXU 15, STATE_ID 0-16    
    #define CX_SELECTOR_USER  0x011
    #define CX_PREV_SELECTOR_USER 0x012
    #define CX_STATUS         0x801
#else
    #define MCX_ENABLE0  0x012
    #define MCX_ENABLE1  0x013
    #define MCX_ENABLE2  0x014
    #define MCX_ENABLE3  0x015

    #define CX_SELECTOR_USER      0x800 
    // #define CX_INDEX      0x800 
    
    #define CX_STATUS     0x801
#endif 

#define NUM_CXUS 6

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

/* CX_SELECTOR_USER */
#define CX_CXU_ID_START_INDEX 0
#define CX_CXU_ID_BITS 4

#define CX_VIRT_STATE_START_INDEX 10
#define CX_VIRT_STATE_BITS 6

#define CX_STATE_ID_START_INDEX 16
#define CX_STATE_ID_BITS 4

#define CX_VER_START_INDEX 28
#define CX_VER_BITS 3

#define CX_IV_START_INDEX 31
#define CX_IV_BITS 1

/* CX_STATUS */
#define CX_IS_BITS 1
#define CX_IS_START_INDEX 0

#define CX_IF_BITS 1
#define CX_IF_START_INDEX 1

/* CX_STATE_CONTEXT_STATUS */
#define CX_STATUS_START_INDEX 0
#define CX_STATUS_BITS 2

#define CX_STATE_SIZE_START_INDEX 0
#define CX_STATE_SIZE_BITS 16

#define CX_DATA_CLEAN_START_INDEX 26
#define CX_DATA_CLEAN_BITS 2

#define CX_STATUS_VERSION_START_INDEX 28

#define CX_RESET_START_INDEX 31
#define CX_RESET_BITS 1

#define CX_VERSION_ID_BITS 3

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

#define GET_CX_STATUS_VERSION(cx_sel) \
    GET_BITS(cx_sel, CX_STATUS_VERSION_START_INDEX, CX_VERSION_ID_BITS)

#define GET_CX_DATA_CLEAN(cx_sel) \
    GET_BITS(cx_sel, CX_DATA_CLEAN_START_INDEX, CX_DATA_CLEAN_BITS)

#define GET_CX_STATE_SIZE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_SIZE_START_INDEX, CX_STATE_SIZE_BITS)

#define GET_CX_RESET(cx_sel) \
    GET_BITS(cx_sel, CX_RESET_START_INDEX, CX_RESET_BITS)

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
    CX_PRECLEAN, 
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
        uint reserved0  : 6;
        uint v_state_id : CX_VIRT_STATE_BITS;
        uint state_id   : CX_STATE_ID_BITS;
        uint reserved1  : 8;
        uint version    : 3;
        uint iv         : 1;
    } sel;
        int idx;
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
        uint state_size  : CX_STATE_SIZE_BITS;
        uint reserved0   : 10;
        uint dc          : CX_DATA_CLEAN_BITS;
        uint version     : CX_VERSION_ID_BITS;
        uint R           : CX_RESET_BITS;
     } sel;
      int idx;
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