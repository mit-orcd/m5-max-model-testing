#include <stddef.h>
#include <stdlib.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out) {
    long long *pre = malloc((n + 1) * sizeof *pre);
    pre[0] = 0;
    for (size_t i = 0; i < n; i++)
        pre[i + 1] = pre[i] + a[i];
    for (size_t i = 0; i < q; i++)
        out[i] = pre[hi[i] + 1] - pre[lo[i]];
    free(pre);
}
