#include <stddef.h>

void range_sums(const int *a, size_t n,
                const size_t *lo, const size_t *hi, size_t q,
                long long *out)
{
    /* Allocate prefix sums array */
    long long *pref = (long long *)malloc((n + 1) * sizeof(long long));
    if (!pref) return; /* allocation failed: nothing to do */

    pref[0] = 0;
    for (size_t i = 0; i < n; ++i)
        pref[i + 1] = pref[i] + (long long)a[i];

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        out[i] = pref[h + 1] - pref[l];
    }

    free(pref);
}