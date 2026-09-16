#include <stdio.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int i = 0;
    int j = 0;
    int neg = 0;

    while (s[i] != '\0') {
        if (s[i] == '-') {
            neg = 1;
            i++;
        }

        if (isdigit(s[i])) {
            int val = 0;
            while (isdigit(s[i])) {
                val = val * 10 + (s[i++] - '0');
            }

            if (j < max) {
                out[j++] = neg ? -val : val;
            }
        }

        if (s[i] == ',') {
            i++;
        } else {
            break;
        }
    }

    return j;
}