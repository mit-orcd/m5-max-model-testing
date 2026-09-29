int binary_search(const int *a, int n, int key)
{
    if (n <= 0 || a == NULL)
        return -1;

    int low = 0;
    int high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (a[mid] == key)
            return mid;
        else if (key < a[mid])
            high = mid - 1;
        else
            low = mid + 1;
    }

    return -1; /* key not found */
}