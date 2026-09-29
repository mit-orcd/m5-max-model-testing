#include <stdlib.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    if (s == NULL || out == NULL || max <= 0) return 0;

    int count = 0;
    const char *p = s;

    while (count < max) {
        /* Parse optional leading minus */
        int neg = 0;
        if (*p == '-') {
            neg = 1;
            p++;
        }

        /* Parse digits */
        long val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }

        out[count++] = neg ? (int)(-val) : (int)val;

        /* If next char is comma, consume it and continue; else stop */
        if (*p == ',') {
            p++;
        } else {
            break;
        }
    }

    return count;
}