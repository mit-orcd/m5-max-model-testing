#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int sign = 1;
    int value = 0;
    int in_number = 0;

    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    while (*s != '\0' && count < max) {
        if (isdigit((unsigned char)*s)) {
            value = value * 10 + (*s - '0');
            in_number = 1;
        } else if (*s == '-' && !in_number) {
            sign = -1;
            in_number = 1;
        } else if (*s == ',') {
            if (in_number) {
                if (count < max) {
                    out[count++] = sign * value;
                }
                sign = 1;
                value = 0;
                in_number = 0;
            }
        }

        if (!isdigit((unsigned char)*s) && *s != '-' && *s != ',') {
            // Skip unexpected characters, though input is guaranteed well-formed
        }

        s++;
    }

    if (in_number) {
        if (count < max) {
            out[count++] = sign * value;
        }
    }

    return count;
}