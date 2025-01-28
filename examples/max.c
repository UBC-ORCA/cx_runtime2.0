#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/ci.h"
#include "../zoo/max/max.h"

void test() {
    int a = 3, b = 5, res = 0;

    cx_select_t selM = cx_open(CX_GUID_MAX, CX_NO_VIRT, -1);
    assert(selM > 0);

    cx_sel(selM);

    res = max(0, a);
    assert(res == a);

    res = max(b, 0);
    assert(res == b);

    res = max(a, b);
    assert(res == 5);

    res = max(-a, a);
    assert(res == a);

    res = max(-b, 0);
    assert(res == 0);

    return;
}

int main() {
    test();
    printf("Max test passed!\n");
    return 0;
}