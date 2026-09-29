#include <stdint.h>

int cmp_desc(const void *a, const void *b) {
    int va = *(const int *)a;
    int vb = *(const int *)b;

    if (va > 0 && vb > 0) {
        return vb - va;
    } else if (va < 0 && vb < 0) {
        return vb - va;
    } else {
        return (va > vb) ? 1 : (va < vb) ? -1 : 0;
    }
}