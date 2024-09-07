#include <stdio.h>

int main() {
    int a = 2;
    int b = 3;
    int c;
    asm volatile(
        "cx_reg 0, %0, %1, %2"
        : "=r"(c)
        : "r"(a), "r"(b)
        :
    );
    printf("c: %d\n", c);
    return 0;
}