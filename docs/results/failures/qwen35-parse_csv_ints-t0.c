#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int sign = 1;
    long value = 0;
    int in_number = 0;

    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    while (*s && count < max) {
        if (isdigit((unsigned char)*s)) {
            if (!in_number) {
                in_number = 1;
                value = 0;
                sign = 1;
            }
            value = value * 10 + (*s - '0');
        } else if (*s == '-' && !in_number) {
            sign = -1;
            in_number = 1;
        } else if (*s == ',') {
            if (in_number) {
                if (count < max) {
                    out[count++] = (int)(sign * value);
                }
                in_number = 0;
                value = 0;
            }
        } else {
            // Skip any other characters or handle as per well-formed assumption
            if (in_number) {
                if (count < max) {
                    out[count++] = (int)(sign * value);
                }
                in_number = 0;
                value = 0;
            }
        }
        s++;
    }

    if (in_number) {
        if (count < max) {
            out[count++] = (int)(sign * value);
        }
    }

    return count;
}