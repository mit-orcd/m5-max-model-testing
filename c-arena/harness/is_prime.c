
#include <stdio.h>
int is_prime(int n);
static int fails = 0;
static void check(int n, int want) {
    int got = is_prime(n);
    if ((got != 0) != (want != 0)) { printf("FAIL is_prime(%d)=%d want %d\n", n, got, want); fails++; }
}
int main(void) {
    check(2,1); check(3,1); check(5,1); check(7,1); check(97,1); check(7919,1);
    check(0,0); check(1,0); check(-3,0); check(4,0); check(9,0); check(91,0); check(100,0);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

