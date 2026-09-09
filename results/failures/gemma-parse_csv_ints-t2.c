#include <stdio.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (*p != '\0' && count < max) {
        // Skip leading commas if they exist (though input is well-formed)
        if (*p == ',') {
            p++;
            continue;
        }

        // Parse integer
        char *endptr;
        int val = (int)strtol(p, &endptr, 10);

        // If no digits were consumed, break
        if (p == endptr) {
            break;
        }

        out[count++] = val;
        p = endptr;

        // If the next char is a comma, skip it to move to next number
        if (*p == ',') {
            p++;
        }
    }

    return count;
}