#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out)
{
    /* Allocate prefix sum array on the stack if size permits,
       otherwise use dynamic allocation. */
    long long *pref;
    if (n + 1 <= 1024) {
        long long stack_pref[1025]; /* n+1 <= 1025 */
        pref = stack_pref;
    } else {
        pref = (long long *)malloc((n + 1) * sizeof(long long));
        if (!pref) {
            /* Handle allocation failure: set outputs to zero. */
            for (size_t i = 0; i < q; ++i)
                out[i] = 0;
            return;
        }
    }

    pref[0] = 0LL;
    for (size_t i = 0; i < n; ++i)
        pref[i + 1] = pref[i] + (long long)a[i];

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        out[i] = pref[h + 1] - pref[l];
    }

    if (pref != (long long (*)[1025])pref) /* not stack */
        free(pref);
}