#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    int sign = 1;
    long long val = 0;
    int has_digit = 0;

    while (*p && count < max) {
        if (isdigit((unsigned char)*p)) {
            if (!has_digit) {
                sign = 1;
                val = 0;
                has_digit = 1;
            }
            val = val * 10 + (*p - '0');
            p++;
        } else if (*p == '-' && !has_digit) {
            sign = -1;
            p++;
        } else if (*p == ',' || *p == '\0') {
            if (has_digit) {
                out[count++] = (int)(val * sign);
                has_digit = 0;
            }
            if (*p == '\0') break;
            p++;
        } else {
            p++;
        }
    }

    return count;
}