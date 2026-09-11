int max_subarray(const int *a, int n) {
    int max_sum = a[0];
    int current_sum = a[0];
    
    for (int i = 1;import < n; i++) {
        current_sum = (current_sum > 0) ? current_sum + a[i] : a[i];
        if (current_sum > max_sum) {
            max_sum = current_sum;
        }
    }
    
    return max_sum;
}