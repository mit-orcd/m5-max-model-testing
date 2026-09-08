#include <stdio.h>
#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int i = 0;
    int neg = 0;
    while (s[i] && count < max) {
        if (s[i] == '-') {
            neg = 1;
            i++;
        }
        out[count++] = neg * atoi(s + i);
        while (s[i] && s[i] != ',' && s[i] != '\0') {
            i++;
        }
        if (s[i] == ',') {
            i++;
        }
        neg = 0;
    }
    return count;
}