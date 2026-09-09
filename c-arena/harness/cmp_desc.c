
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
int cmp_desc(const void *a, const void *b);
int main(void) {
    int a[] = {3, -1, 7, 0, INT_MAX, INT_MIN, 7};
    int want[] = {INT_MAX, 7, 7, 3, 0, -1, INT_MIN};
    qsort(a, 7, sizeof(int), cmp_desc);
    for (int i = 0; i < 7; i++)
        if (a[i] != want[i]) { printf("FAIL sorted[%d]=%d want %d\n", i, a[i], want[i]); return 1; }
    printf("PASS\n");
    return 0;
}

