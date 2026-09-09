
#include <stdio.h>
int max_subarray(const int *a, int n);
static int fails = 0;
static void check(const int *a, int n, int want) {
    int got = max_subarray(a, n);
    if (got != want) { printf("FAIL max_subarray got %d want %d\n", got, want); fails++; }
}
int main(void) {
    int t1[] = {-2,1,-3,4,-1,2,1,-5,4}; int t2[] = {-3,-1,-2};
    int t3[] = {5}; int t4[] = {1,2,3,4}; int t5[] = {2,-1,2};
    check(t1,9,6); check(t2,3,-1); check(t3,1,5); check(t4,4,10); check(t5,3,3);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

