#include <limits.h>

int cmp_desc(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    if (ia > ib) {
        if (ia > 0 && ib < 0) return 1;
        if (ia < 0 && ib < 0) {
            if (ia - ib > 0) return 1;
            if (ia - ib < 0) return -1;
            return 0;
        }
        return 1;
    } else if (ia < ib) {
        if (ia < 0 && ib > 0) return -1;
        if (ia < 0 && ib < 0) {
            if (ib - ia > 0) return -1;
            if (ib - ia < 0) return 1;
            return 0;
        }
        return -1;
    }
    return 0;
}