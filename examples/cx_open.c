#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/mulacc/mulacc.h"

int a = 5, b = 3, result = 0;
void exclusive_open() {
    cx_select_t sel = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    assert(sel > 0);
    cx_close(sel);
}

void inter_open() {
    cx_select_t sel = cx_open(CX_GUID_MULACC, CX_INTER_VIRT, -1);
    assert(sel > 0);
    cx_close(sel);
}

void exclusive_open_2() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    assert(selA > 0);
    assert(selB > 0);
    cx_close(selA);
    cx_close(selB);
}

void exclusive_open_3() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selC = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    printf("opens finished\n");
    assert(selA > 0);
    assert(selB > 0);
    assert(selC < 0);
        
    cx_sel(selA);
    result = mac(a, a);
    assert(result == 25);

    cx_sel(selB);
    result = mac(b, b);
    assert(result == 9);

    cx_sel(selA);
    result = mac(a, b);
    assert(result == 40);

    cx_sel(selB);
    result = mac(a, b);
    assert(result == 24);

    cx_close(selA);
    cx_close(selB);
}

void exclusive_open_4() {
    cx_select_t selA = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    cx_select_t selB = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    assert(selA > 0);
    assert(selB > 0);
    cx_sel( selA );

    result = mac(b, b);
    assert(result == 9);

    cx_close(selA);
    cx_close(selB);

    cx_sel(CX_LEGACY);
}

void mixed_open_1() {
    cx_select_t sel = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    assert(sel > 0);

    cx_sel(sel);
    result = mac(a, a);
    assert(result == 25);

    cx_close(sel);
    sel = cx_open(CX_GUID_MULACC, CX_NO_VIRT, -1);
    assert(sel > 0);
    cx_close(sel); 
}

int main() {
    // exclusive_open();
    // inter_open();
    // exclusive_open_2();
    exclusive_open_3();
    // exclusive_open_4();
    // mixed_open_1();
    // percounter_read();
    printf("cx_open test passed!\n");
    return 0;
}
