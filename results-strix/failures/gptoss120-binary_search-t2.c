int binary_search(const int *a, int n, int key)
{
    if (a == NULL || n <= 0)
        return -1;

    int lo = 0;
    int hi = n - 1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = a[mid];

        if (val == key)
            return mid;
        else if (val < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }

    return -1;
}