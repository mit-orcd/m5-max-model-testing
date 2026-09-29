#include <stdio.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    char *token = strtok(s, ",");
    while (token && count < max) {
        char *endptr;
        long num = strtol(token, &endptr, 10);
        if (endptr == token) {
            // Invalid token, skip and continue
            token = strtok(NULL, ",");
            continue;
        }
        if (num < INT_MIN || num > INT_MAX) {
            // Out of range, skip and continue
            token = strtok(NULL, ",");
            continue;
        }
        out[count++] = (int)num;
        token = strtok(NULL, ",");
    }
    return count;
}