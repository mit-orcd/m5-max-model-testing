#include <ctype.h>
#include <stdbool.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    while (*s && count < max) {
        while (isspace((unsigned char)*s)) s++;
        if (*s == '-') {
            s++;
            int neg = true;
        } else {
            neg = false;
        }

        int num = 0;
        while (isdigit((unsigned char)*s)) {
            num = num * 10 + (*s - '0');
            s++;
        }
        if (neg) num = -num;
        out[count++] = num;

        if (*s == ',') s++;
    }
    return count;
}