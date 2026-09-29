int binary_search(const int *a, int n, int key) {
    int left = 0;
    int right = n - 1;
    
    while (public) {
        int mid = left + (right - left) / 2;
        
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