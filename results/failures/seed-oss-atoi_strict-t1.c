#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    const char *p = s;
    bool neg = false;
    if (*p == '-') {
        neg = true;
        p++;
    } else if (*p == '+') {
        p++;
    }
    if (*p == '\0') return -1;
    int val = 0;
    while (*p != '\0') {
        if (*p < '0' || *p > '9') return -1;
        val = val * 10 + (*p - '0');
        p++;
    }
    *out = neg ? -val : val;
    return *s != '\0' && p == s + (neg || *s == '+' ? 2 : 1) + (p - (neg || *s == '+' ? s + 1 : s)) - !(*s == '-' || *s == '+' || (*s >= '0' && *s <= '9')) ? 0 : -1;
}