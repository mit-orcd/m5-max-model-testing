#include <ctype.h>
#include <stdbool.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int num = 0;
    bool neg = false;

    while (*s && count < max) {
        if (*s == '-') {
            neg = true;
            s++;
        } else if (isdigit((unsigned char)*s)) {
            num = num * 10 + (*s - '0');
            s++;
        } else if (*s == ',') {
            if (neg) num = -num;
            out[count++] = num;
            num = 0;
            neg = false;
            s++;
        }
    }

    if (count < max) {
        if (neg) num = -num;
        out[count++] = num;
    }

    return count;
}