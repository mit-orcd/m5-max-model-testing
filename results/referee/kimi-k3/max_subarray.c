int max_subarray(const int *a, int n) {
    int best = a[0], cur = a[0];
    for (int i = 1; i < n; i++) {
        cur = cur > 0 ? cur + a[i] : a[i];
        if (cur > best) best = cur;
    }
    return best;
}
