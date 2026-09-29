#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0, i = 0;
    int sign = 1;
    while (s[i] != '\0') {
        if (s[i] == '-') {
            sign = -1;
            i++;
        }
        if (isdigit(s[i])) {
            int num = 0;
            while (isdigit(s[i])) {
                num = num * 10 + (s[i++] - '0');
            }
            if (count < max) {
                out[count++] = num * sign;
            }
        }
        if (s[i] == ',' || s[i] == '\0') {
            i++;
        }
    }
    return count;
}