#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/mulacc/mulacc.h"

int main() {
    int a = 3;
    int b = 5;
    int result;

    // int cx_sel_A = cx_open(CX_GUID_A, share_A);
    cx_csr_write(CX_INDEX, CX_LEGACY);
    int cx_sel_C0 = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    int cx_sel_C1 = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    int cx_sel_C2 = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);


    assert ( cx_sel_C0 != -1 );
    assert ( cx_sel_C1 != -1 );
    assert ( cx_sel_C2 != -1 );

    printf("cx_sel_0: %08x, cx_sel_1: %08x, cx_sel_2: %08x\n", cx_sel_C0, cx_sel_C1, cx_sel_C2);

    // printf("cx_sel_C0: %d, cx_sel_C: %d\n", cx_sel_C0, cx_sel_C1);
    // printf("a: %d, b: %d\n", a, b);

    cx_sel(cx_sel_C0);
    result = mac(a, a);
    printf("result (mulacc: a * a, state_id: 0) (9): %d\n", result);

    cx_sel(cx_sel_C1);
    result = mac(b, b);
    printf("result (mulacc: b * b, state_id: 1) (25): %d\n", result);

    cx_sel(cx_sel_C0);
    result = mac(a, a);
    printf("result (mulacc: a * a, state_id: 0) (18): %d\n", result);

    cx_sel(cx_sel_C1);
    result = mac(a, b);
    printf("result (mulacc: a * b, state_id: 1) (40): %d\n", result);

    cx_sel(cx_sel_C2);
    result = mac(a, b);
    printf("result (mulacc: a * b, state_id: 0, vid: 1) (15): %d\n", result);

    cx_sel(cx_sel_C0);
    result = mac(a, a);
    printf("result (mulacc: a * a, state_id: 0) (27): %d\n", result);

    cx_sel(cx_sel_C2);
    result = mac(a, b);
    printf("result (mulacc: a * b, state_id: 0, vid: 1) (30): %d\n", result);

    cx_sel(cx_sel_C1);
    result = mac(a, b);
    printf("result (mulacc: a * b, state_id: 1) (55): %d\n", result);

    cx_close(cx_sel_C2);
    cx_close(cx_sel_C1);
    cx_close(cx_sel_C0);

    printf("completed!\n");
    return 0;
}
