#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sums: prefix[i] = sum of a[0]..a[i-1], prefix[0] = 0
    // To save memory and avoid dynamic allocation, we can compute sums on the fly
    // or use a local array if n is small, but n can be large.
    // However, we must allocate prefix sums array. Since we cannot assume malloc is available 
    // or desired, and the signature doesn't allow extra parameters, we have two choices:
    // 1. Use O(n) extra space via malloc (requires #include <stdlib.h> and potential failure handling)
    // 2. Answer each query in O(n) time, leading to O(n*q) total time.
    //
    // Given no constraints on n and q, and that this is a typical competitive programming-style 
    // function, prefix sums with malloc is standard. But the problem says "Implement in C11" 
    // and gives a specific signature. Let's use malloc for prefix sums.
    
    // Edge case: if n is 0, all queries should yield 0 (but hi[i] < n implies n > 0 if q > 0)
    // If q is 0, nothing to do.
    
    if (q == 0 || n == 0) {
        // If n==0, there are no valid elements, but queries are guaranteed hi[i] < n, so q must be 0.
        return;
    }
    
    // Allocate prefix sum array of size n+1
    long long *prefix = (long long *)calloc(n + 1, sizeof(long long));
    if (!prefix) {
        // In case of allocation failure, we cannot proceed correctly. 
        // For robustness, we might fallback or leave out uninitialized. 
        // Given the problem statement, we assume memory is available.
        return;
    }
    
    // Build prefix sums
    // prefix[0] = 0
    // prefix[k] = a[0] + a[1] + ... + a[k-1]
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    // Answer each query
    for (size_t i = 0; i < q; i++) {
        // Sum of a[lo[i]..hi[i]] = prefix[hi[i]+1] - prefix[lo[i]]
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}