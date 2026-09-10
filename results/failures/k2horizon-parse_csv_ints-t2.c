#include <stddef>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (count < max && *p != '\0') {
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        }

        int value = 0;
        while (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            ++p;
        }

        out[count] = sign * value;
        ++count;

        if (*p == ',') {
            ++p;
        }
    }

    return count;
}