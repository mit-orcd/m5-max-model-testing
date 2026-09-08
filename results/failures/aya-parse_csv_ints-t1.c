#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int i, j, count = 0;
    int negative = 0;
    char num[12];

    for (i = 0; s[i] != '\0'; i++) {
        if (s[i] == '-') {
            negative = 1;
            i++;
        }
        if (isdigit(s[i])) {
            j = 0;
            while (isdigit(s[i]) && j < 11) {
                num[j++] = s[i++];
            }
            num[j] = '\0';
            if (count < max) {
                out[count++] = atoi(num) * (negative ? -1 : 1);
            }
            negative = 0;
        }
        if (s[i] == ',') {
            i++;
        }
    }
    return count;
}