int max_subarray(const int *a, int n) {
    int max_so_far = a[0];
    int current_sum = a[0];

    for (int i = 1; i < n; i++) {
        current_sum = fmax(current_sum + a[i], a[i]);
        max_so_far = fmax(max_so_far, current_sum);
    }

    return max_so_far;
}