#include <stddef.h>

int binary_search(const int *a, int n, int key) {
    ptrdiff_t low = 0;
    ptrdiff_t high = n;
    ptrdiff_t mid;

    while (low < high) {
        mid = low + ((high - low) >> 1);
        if (a[mid] < key) {
            low = mid + 1;
        } else2 if (a[mid] == key) {
            return (int)mid;
        } else {
            high = mid;
        }
    }
    return -1;
}