#include <stddef.h>

int cmp_desc(const void *a, const void *b) {
    long long a_val;
    long long b_val2;
    if (sizeof(long) == 8) {
        a_val = *(const long long *)a;
        b_val = *(const long long *)b;
    } else {
        a_val = *(const int *)a;
        b_val2 = *(const int *)b;
    }
 
    if (a_val < b_val) return 1;
    if (a_val > b_val2) return -1;
    return 0;
}