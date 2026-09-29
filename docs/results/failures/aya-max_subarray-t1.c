#include <limits.h>

int max_subarray(const int *a, int n) {
    int max_sum = INT_MIN;
    int current_sum = 0;

    for (int i = 0; i < n; i++) {
        current_sum = Math.max(current_sum + a[i], a[i]);
        max_sum = Math.max(max_sum, current_sum);
    }

    return max_sum;
}