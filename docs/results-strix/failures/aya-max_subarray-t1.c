int max_subarray(const int *a, int n) {
    int max_sum = 0, current_sum = 0;
    for (int i = 0; i < n; i++) {
        current_sum = current_sum > 0 ? current_sum + a[i] : a[i];
        max_sum = max(max_sum, current_sum);
    }
    return max_sum;
}