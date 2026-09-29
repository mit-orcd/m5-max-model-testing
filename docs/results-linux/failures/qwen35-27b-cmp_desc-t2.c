#include <stdlib.h>

int cmp_desc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    int x = *ia;
    int y = *ib;

    if (x == y) {
        return 0;
    }

    if ((x < 0 && y >= 0) || (x >= 0 && y < 0)) {
        return (x < 0) ? -1 : 1;
    }

    if (x > 0 && y > 0) {
        return (y > x) ? -1 : 1;
    }

    if (x < 0 && y < 0) {
        return (y > x) ? 1 : -1;
    }

    return 0;
}