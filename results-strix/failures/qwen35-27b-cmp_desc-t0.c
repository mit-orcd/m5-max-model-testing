#include <stdlib.h>

int cmp_desc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    int va = *ia;
    int vb = *ib;

    if (va > vb) {
        return (va < 0 && vb > 0) ? 1 : (va - vb > 0 ? 1 : -1);
    } else if (va < vb) {
        return (va > 0 && vb < 0) ? -1 : (va - vb < 0 ? -1 : 1);
    }
    return 0;
}