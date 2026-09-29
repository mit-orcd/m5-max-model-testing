#include <limits.h>

int max_subarray(const int *a, int n) {
    int max_sum = 0, sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
        if (sum > max_sum) {
            max_sum = sum;
        }
        if (sum < 0) {
            sum = 0;
        }
    }
    return max_sum;
}