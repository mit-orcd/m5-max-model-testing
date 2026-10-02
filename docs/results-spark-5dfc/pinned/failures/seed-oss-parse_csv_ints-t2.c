#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *current = s;
    while (current != '\0' && count < max) {
        const char *end = current;
        while (*end != ',' && *end != '\0') {
            end++;
        }
        char *endptr;
        int num = strtoi(current, &endptr, 10);
        out[count++] = num;
        current = end;
        if (*current == ',') {
            current++;
        }
    }
    return count;
}