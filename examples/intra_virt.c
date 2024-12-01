#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/mulacc/mulacc.h"

int a = 5, b = 3, res = 0;

void intra_open_1() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    assert(selA > 0);
    assert(selB > 0);
    cx_close(selA);
    cx_close(selB);
}

void intra_open_2() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selC = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selD = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);

    assert(selA > 0);
    assert(selB > 0);
    assert(selC > 0);
    assert(selD > 0);
    cx_close(selA);
    cx_close(selB);
    cx_close(selC);
    cx_close(selD);
}

void intra_open_3() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selC = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t selD = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);

    assert(selA > 0);
    assert(selB > 0);
    assert(selC > 0);
    assert(selD > 0);

    assert(CX_GET_CXU_ID(selA) == CX_GET_CXU_ID(selB));
    assert(CX_GET_CXU_ID(selB) == CX_GET_CXU_ID(selC));
    assert(CX_GET_CXU_ID(selC) == CX_GET_CXU_ID(selD));

    cx_sel(selB);
    res = mac(a, a);
    assert(res == 25);

    cx_sel(selC);
    res = mac(b, b);
    assert(res == 9);

    cx_sel(selD);
    res = mac(a, b);
    assert(res == 15);
    
    cx_sel(selB);
    res = mac(b, b);
    assert(res == 34);

    cx_sel(selC);
    res = mac(b, b);
    assert(res == 18);

    cx_sel(selD);
    res = mac(b, a);
    assert(res == 30);

    cx_sel(selB);
    res = mac(b, b);
    assert(res == 43);

    cx_close(selA);
    cx_close(selB);
    cx_close(selC);
    cx_close(selD);
}

void intra_open_4() {
    // cx_sel( CX_LEGACY );
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    assert(selA > 0);
    assert(selB > 0);

    cx_sel(selA);
    res = mac(a, a);
    assert(res == 25);

    cx_select_t selC = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    assert(CX_READ_STATE(0) == 25);

    cx_sel(selC);
    assert(CX_READ_STATE(0) == 0);

    cx_close(selA);
    cx_close(selB);
    cx_close(selC);
}

// Changing the initialization to system land can break this test.
void intra_open_5() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    assert(selB > 0);

    cx_sel(selB);
    res = mac(a, a);
    assert(res == 25);

    cx_select_t selC = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    assert(CX_READ_STATE(0) == 25);
    assert(selC > 0);
    assert(selB == cx_csr_read(CX_SELECTOR_USER));

    res = mac(a, b);
    assert(res == 40);

    cx_sel(selC);
    assert(CX_READ_STATE(0) == 0);
    res = mac(a, b);
    assert(res == 15);

    cx_close(selA);
    cx_close(selB); 
    cx_close(selC); 
}

int main() {
    intra_open_1();
    intra_open_2();
    intra_open_3();
    intra_open_4();
    intra_open_5();
    printf("intra virt test passed!\n");
    return 0;
}
