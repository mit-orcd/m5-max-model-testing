#include <stdint.h>

int cmp_desc(const void *a, const void *b)
{
    const int *pa = (const int *)a;
    const int *pb = (const int *)b;
    int x = *pa;
    int y = *pb;

    /* Compare descending: return >0 if x > y, <0 if x < y, 0 if x == y.
       Safe against overflow: use sign comparison on difference via
       (x - y) would overflow; instead compare by signs and magnitudes. */
    if (x == y)
        return 0;

    /* Use the fact that for signed integers, we can compare without
       subtracting by checking the sign of the difference indirectly.
       A robust method: compare as if unsigned after adjusting for sign. */

    /* Method: 
       If x > y, return 1; if x < y, return -1.
       To avoid overflow, we can use:
         (x > y) ? 1 : -1
       which is safe.
    */
    return (x > y) ? 1 : -1;
}