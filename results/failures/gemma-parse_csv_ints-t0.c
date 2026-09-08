#include <stdio.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (*p != '\0' && count < max) {
        // Skip leading commas if any (though input is well-formed)
        if (*p == ',') {
            p++;
            continue;
        }

        // Parse integer
        char *endptr;
        int val = (int)strtol(p, &endptr, 10);

        // If no digits were found, strtol sets endptr to p
        if (endptr == p) {
            // If we encounter a comma immediately, move past it
            if (*p == ',') {
                p++;
                continue;
            }
            break;
        }

        out[count++] = val;
        p = endptr;

        // If next char is a comma, skip it to prepare for next iteration
        if (*p == ',') {
            p++;
        }
    }

    return count;
}