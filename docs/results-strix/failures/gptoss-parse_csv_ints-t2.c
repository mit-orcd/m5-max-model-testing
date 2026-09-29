#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    while (*p != '\0') {
        char *end;
        long val = strtol(p, &end, 10);
        if (count < max) {
            out[count] = (int)val;
        }
        count++;
        p = end;
        if (*p == ',') {
            ++p;
        }
    }
    return count;
}