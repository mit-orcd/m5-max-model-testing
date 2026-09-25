#include <ctype.h>
#include <stdbool.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    bool negative = false;
    int num = 0;

    while (*s && count < max) {
        if (*s == '-') {
            negative = true;
            s++;
        } else if (*s == ',') {
            if (negative) num = -num;
            out[count++] = num;
            num = 0;
            negative = false;
            s++;
        } else {
            num = num * 10 + (*s - '0');
            s++;
        }
    }

    if (count < max) {
        if (negative) num = -num;
        out[count++] = num;
    }

    return count;
}