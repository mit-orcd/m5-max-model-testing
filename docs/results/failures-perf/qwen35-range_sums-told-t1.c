#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }

    size_t* prefix_sum = NULL;
    if (n > 0) {
        prefix_sum = (size_t*)malloc((n + 1) * sizeof(size_t));
        if (prefix_sum == NULL) {
            return;
        }
        
        prefix_sum[0] = 0;
        for (size_t i = 0; i < n; ++i) {
            // Cast to long long to prevent overflow during accumulation, 
            // though the array is int, the sum can exceed 32-bit int.
            prefix_sum[i + 1] = prefix_sum[i] + (size_t)a[i]; 
            // Note: The problem implies a[i] are ints, but sums can exceed 32-bit.
            // However, the signature uses size_t for prefix sums. 
            // If the sum exceeds 2^64, this approach fails, but typically 
            // competitive programming constraints assume it fits in available types 
            // or we cast the input to long long for accumulation.
            // Given the return type is long long, let's adjust prefix_sum to hold long long 
            // to be safe, but we cannot change the function signature.
            // We must assume the sums fit in size_t or cast carefully.
            // Re-reading: "long long *out" suggests sums can be large. 
            // Using size_t for prefix array might overflow if sum > SIZE_MAX.
            // To be safe and strictly correct given the output type, we should use long long for the prefix array.
            // But reallocating a prefix array of size_t is what we have here.
            // Let's fix the type of the internal array to long long to prevent overflow issues 
            // before the final subtraction, assuming n fits in size_t.
        }
        // Wait, I allocated size_t array. Let's reallocate with long long to be safe.
        // Since I cannot change the signature, I must handle the overflow risk internally.
        // Correct approach: allocate long long array for prefix sums.
    }
    
    // Re-implementing with correct types for safety
    free(prefix_sum); // Free if allocated (though logic above was flawed for type safety)
    
    // Allocate long long prefix array to handle potential large sums
    long long *p_sum = (long long*)malloc((n + 1) * sizeof(long long));
    if (p_sum == NULL) {
        return;
    }

    p_sum[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        p_sum[i + 1] = p_sum[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        // Sum from l to h inclusive is p_sum[h+1] - p_sum[l]
        out[i] = p_sum[h + 1] - p_sum[l];
    }

    free(p_sum);
}