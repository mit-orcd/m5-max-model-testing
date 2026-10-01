int max_subarray(const int *a, int n) {
    int max_sum = a[0];
    int current_sum = max_sum;

    for (int i = 1; i < n; i++) {
        current_sum = fmax(current_sum + a[i], a[i]);
        max_sum = fmax(max_sum, current_sum);
    }

    return max_sum;
}