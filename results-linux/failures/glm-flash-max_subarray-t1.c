#include <limits.h>

int max_subarray(const int *a, int n) {
    if (a == NULL || n <= 0) {
        return INT_MIN;
    }

    int max_ending_here = a[0];
    int max_so_far = a[0];

    for (int i = 1; i < n; i++) {
        max_ending_here = (a[i] > max_ending_here + a[i]) ? a[i] : max_ending_here + a[i];
        if (max_ending_here > max_so_far) {
            max_so_far = max_ending_here;
        }
    }

    return max_so_far;
}