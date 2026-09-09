#include <assert.h>

int max_subarray(const int *a, int n)
{
    assert(a != NULL && n >= 1);

    int best = a[0];
    int cur  = a[0];

    for (int i = 1; i < n; ++i) {
        cur = (cur > 0) ? cur + a[i] : a[i];
        if (cur > best)
            best = cur;
    }
    return best;
}