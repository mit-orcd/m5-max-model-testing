
#include <stdio.h>
unsigned popcount(unsigned x);
static int fails = 0;
static void check(unsigned x, unsigned want) {
    unsigned got = popcount(x);
    if (got != want) { printf("FAIL popcount(%u)=%u want %u\n", x, got, want); fails++; }
}
int main(void) {
    check(0,0); check(1,1); check(0xFFFFFFFFu,32); check(0xF0F0F0F0u,16);
    check(7,3); check(0x80000000u,1); check(123456789u,16);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

