#include <stdio.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int i, j, count = 0;
    int sign = 1;
    for (i = 0; s[i] != '\0'; i++) {
        if (s[i] == '-') {
            sign = -1;
            i++;
        }
        if (isdigit(s[i])) {
            int val = 0;
            for (j = i; s[j] != ',' && s[j] != '\0'; j++) {
                val = val * 10 + (s[j] - '0');
            }
            out[count++] = val * sign;
            i = j;
        }
    }
    return count;
}