#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/vector/vector.h"

#define A 0
#define B 1

int main() {
    int result;

    cx_csr_write(CX_INDEX, CX_LEGACY);
    int cx_sel_C0 = cx_open(CX_GUID_VECTOR, CX_NO_VIRT, -1);
    int cx_sel_C1 = cx_open(CX_GUID_VECTOR, CX_NO_VIRT, -1);
    int cx_sel_C2 = cx_open(CX_GUID_VECTOR, CX_NO_VIRT, -1);
    int cx_sel_C3 = cx_open(CX_GUID_VECTOR, CX_NO_VIRT, -1);

    assert ( cx_sel_C0 != -1 );
    assert ( cx_sel_C1 != -1 );
    assert ( cx_sel_C2 != -1 );
    assert ( cx_sel_C3 != -1 );

    printf("cx_sel_0: %08x, cx_sel_1: %08x, cx_sel_2: %08x, cx_sel_3: %08x\n", cx_sel_C0, cx_sel_C1, cx_sel_C2, cx_sel_C3);

    cx_sel(cx_sel_C0);
    set_inc(A);
    // set2(A);
    set2(B);
    
    addv(A, B);

    uint reg_vals_0[16] = {0};
    for (int i = 0; i < 16; i++) {
        reg_vals_0[i] = CX_READ_STATE(i);
        printf("%d ", reg_vals_0[i]);
    }
    printf("\n");

    cx_sel(cx_sel_C1);
    set_inc(A);
    set_inc(B);
    addv(A, B);

    uint reg_vals_1[16] = {0};
    for (int i = 0; i < 16; i++) {
        reg_vals_1[i] = CX_READ_STATE(i);
        printf("%d ", reg_vals_1[i]);
    }
    printf("\n");

    cx_sel(cx_sel_C0);

    for (int i = 0; i < 16; i++) {
        reg_vals_0[i] = CX_READ_STATE(i);
        printf("%d ", reg_vals_0[i]);
    }
    printf("\n");

    cx_sel(cx_sel_C1);
    for (int i = 0; i < 16; i++) {
        reg_vals_1[i] = CX_READ_STATE(i);
        printf("%d ", reg_vals_1[i]);
    }
    printf("\n");

    cx_close(cx_sel_C1);
    cx_close(cx_sel_C0);

    printf("completed!\n");
    return 0;
}
