#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/nn_acc/nn_acc.h"

static const int a = 3, b = 5;
static int res = 0;


void nn_relu_test() {
    cx_select_t selM = cx_open(CX_GUID_NN_ACC, CX_NO_VIRT, -1);
    assert(selM > 0);

    cx_sel(selM);

    CX_WRITE_STATE(0, a);
    res = nn_relu();
    assert(res == a);

    CX_WRITE_STATE(0, b);
    res = nn_relu();
    assert(res == b);

    CX_WRITE_STATE(0, -a);

    res = nn_relu();
    assert(res == 0);

    cx_close(selM);

    return;
}

void mac_test() {
    cx_select_t selM = cx_open(CX_GUID_NN_ACC, CX_NO_VIRT, -1);
    cx_select_t selN = cx_open(CX_GUID_NN_ACC, CX_NO_VIRT, -1);
    assert(selM > 0);
    assert(selN > 0);

    cx_sel(selM);

    res = nn_acc(a, b);
    assert(res == a * b);

    res = nn_acc(a, -a);
    assert(res == a * b - a * a);

    cx_close(selM);
    cx_close(selN);
}

void nn_test() {
    cx_select_t selM = cx_open(CX_GUID_NN_ACC, CX_NO_VIRT, -1);
    cx_sel(selM);

    res = nn_acc(b, -b);
    assert(res == -b * b);

    res = nn_relu();
    assert(res == 0);

    res = nn_relu();
    assert(res == 0);

    res = nn_acc(b, b);
    assert(res == b * b);

    res = nn_relu();
    assert(res == b * b);

    cx_close(selM);
}

int main() {
    cx_sel( CX_LEGACY );
    nn_relu_test();
    mac_test();
    nn_test();
    printf("NN acc test passed!\n");
    return 0;
}