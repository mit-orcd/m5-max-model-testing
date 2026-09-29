int binary_search(const int *a, int n, int key) {
    int left = 0;
    int right = n - 1;
    
    while (int mid = (left + right) / 2; left <= right; ) {
        if (a[mid] == key) {
            return mid;
        } else if (a[mid] < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}