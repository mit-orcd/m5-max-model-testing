#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int i = 0;
    while (s[i] != '\0' && count < max) {
        int j = i;
        while (s[j] != '\0' && s[j] != ',') {
            j++;
        }
        char *endptr;
        int num = (int)strtol(s + i, &endptr, 10);
        out[count++] = num;
        i = j + 1;
    }
    return count;
}