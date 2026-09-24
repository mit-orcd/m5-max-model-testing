#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    // Build prefix sums
    // prefix[i] = sum of a[0] .. a[i-1], so prefix[0] = 0
    // We need prefix array of size n+1
    long long *prefix = NULL;
    // Use stack or dynamic allocation? n can be hundreds of thousands, so stack might be risky.
    // Let's use malloc-free approach if possible, but we need an array.
    // Actually, we can compute prefix sums on the fly if we process queries in order? No, queries are arbitrary.
    // We need prefix sums. Allocate dynamically.
    
    // Alternative: avoid allocation by computing prefix sums into a local buffer if n is small, but n is large.
    // Let's use a VLA or malloc. VLA is C99/C11.
    // long long prefix[n + 1]; // This might cause stack overflow for n in hundreds of thousands (800KB+).
    // Better to use malloc.
    
    long long *pref = (long long *)malloc((n + 1) * sizeof(long long));
    if (!pref) {
        // Fallback or error handling? For competitive programming style, assume success or handle gracefully.
        // If malloc fails, we can't proceed efficiently. But let's assume it works.
        // Actually, let's just do it.
        return;
    }

    pref[0] = 0;
    for (size_t i = 0; i < n; i++) {
        pref[i + 1] = pref[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        // sum from l to r inclusive = pref[r + 1] - pref[l]
        out[i] = pref[r + 1] - pref[l];
    }

    free(pref);
}