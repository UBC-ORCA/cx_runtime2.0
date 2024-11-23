#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/muldiv/muldiv.h"

int main() {
    int a = 3;
    int b = 5;
    int result;

    cx_csr_write(CX_SELECTOR_USER, CX_LEGACY);
    int cx_sel_C0 = cx_open(CX_GUID_MULDIV, CX_NO_VIRT, -1);

    assert ( cx_sel_C0 == 0x80000001 );

    cx_csr_write(CX_SELECTOR_USER, cx_sel_C0);
    result = mul(a, b);
    printf("result (muldiv: a * b, state_id: 0) : %d\n", result);

    cx_close(cx_sel_C0);

    printf("completed!\n");
    return 0;
}
