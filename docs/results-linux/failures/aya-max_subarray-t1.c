int max_subarray(const int *a, int n) {
    int max_so_far = 0, max_ending_here = 0;
    for (int i = 0; i < n; i++) {
        max_ending_here = max_so_far + a[i];
        if (max_ending_here < 0)
            max_ending_here = 0;
        max_so_far = max(max_so_far, max_ending_here);
    }
    return max_so_far;
}