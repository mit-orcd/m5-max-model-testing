#include <stddef.h>
#include <limits.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // We use a sliding window sum technique to achieve O(n + q) time complexity.
    // This is optimal for this problem structure.
    
    size_t left = 0;
    long long current_sum = 0;
    
    // First pass: Initialize the window to the first query range.
    // We scan from 0 to lo[0], accumulating the sum.
    // This handles the case where the first query starts at index 0.
    while (left < lo[0]) {
        current_sum += (long long)a[left];
        left++;
    }
    
    for (size_t i = 0; i < q; i++) {
        size_t r = hi[i];
        
        // Expand the window to the right (r) if necessary
        // This effectively adds elements from left to r (exclusive) to the sum.
        while (left <= r) {
            current_sum += (long long)a[left];
            left++;
        }
        
        // The window currently spans [lo[i], hi[i]].
        // Store the sum in the output array.
        out[i] = current_sum;
        
        // Prepare for the next query
        // If the next query starts before the current left index,
        // we must move the left pointer back, which means subtracting the elements
        // that are being excluded from the new window.
        if (i + 1 < q && lo[i+1] < left) {
            size_t new_lo = lo[i+1];
            // Shrink the window from the left
            while (left > new_lo) {
                left--;
                current_sum -= (long long)a[left];
            }
        }
    }
}