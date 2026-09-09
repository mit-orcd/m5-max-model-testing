
#include <stdio.h>
int binary_search(const int *a, int n, int key);
static int fails = 0;
static void check(const int *a, int n, int key, int want) {
    int got = binary_search(a, n, key);
    if (got != want) { printf("FAIL bsearch key=%d got %d want %d\n", key, got, want); fails++; }
}
int main(void) {
    int a[] = {1,3,5,7,9,11,13}; int one[] = {42};
    check(a,7,1,0); check(a,7,13,6); check(a,7,7,3); check(a,7,4,-1);
    check(a,7,0,-1); check(a,7,99,-1); check(one,1,42,0); check(one,1,1,-1); check(a,0,5,-1);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

