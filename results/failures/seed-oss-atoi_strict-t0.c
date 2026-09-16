#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    const char *p = s;
    bool neg = false;
    if (*p == '-') { neg = true; p++; }
    if (*p == '\0') return -1;
    int val = 0;
    for (; *p; p++) {
        if (*p < '0' || *p > '9') return -1;
        val = val * 10 + (*p - '0');
    }
    if (p != s + (neg ? 2 : 1)) return -1;
    *out = neg ? -val : val;
    return 0;
}