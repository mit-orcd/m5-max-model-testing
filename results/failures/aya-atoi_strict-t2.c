#include <stdio.h>
#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    bool negative = false;
    int i = 0;
    while (s[i] == ' ') i++;
    if (s[i] == '-') {
        negative = true;
        i++;
    }
    if (s[i] < '0' || s[i] > '9') return -1;
    while (s[i] >= '0' && s[i] <= '9') {
        *out = (*out * 10) + (s[i++] - '0');
    }
    if (s[i] != '\0') return -1;
    if (negative) *out = -*out;
    return 0;
}