#include <ctype.h>
#include <stdbool.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    bool negative = false;
    int value = 0;

    while (*s && count < max) {
        if (*s == '-') {
            negative = true;
            s++;
        } else if (*s == ',') {
            if (negative) value = -value;
            out[count++] = value;
            value = 0;
            negative = false;
            s++;
        } else {
            value = value * 10 + (*s - '0');
            s++;
        }
    }

    if (count < max) {
        if (negative) value = -value;
        out[count++] = value;
    }

    return count;
}