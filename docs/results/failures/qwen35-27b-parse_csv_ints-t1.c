#include <ctype.h>
#include <limits.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    const char *p = s;
    while (*p && count < max) {
        if (*p == ',') {
            p++;
            continue;
        }

        int sign = 1;
        long val = 0;
        int digit_seen = 0;

        if (*p == '-') {
            sign = -1;
            p++;
        }

        while (isdigit((unsigned char)*p)) {
            digit_seen = 1;
            val = val * 10 + (*p - '0');
            p++;
        }

        if (!digit_seen) {
            return count;
        }

        long res = sign * val;
        if (res < INT_MIN || res > INT_MAX) {
            res = (sign < 0) ? INT_MIN : INT_MAX;
        }

        out[count++] = (int)res;

        if (*p == ',') {
            p++;
        } else {
            break;
        }
    }

    return count;
}