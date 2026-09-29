#include <string.h>
#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int i = 0;
    int negative = 0;
    while (s[i] != '\0') {
        if (s[i] == '-') {
            negative = 1;
            i++;
        }
        if (isdigit(s[i])) {
            int j = i;
            while (isdigit(s[j])) {
                j++;
            }
            if (j - i > 0) {
                char *end;
                long num = strtol(s + i, &end, 10);
                if (end == s + j && num >= INT_MIN && num <= INT_MAX) {
                    out[count++] = negative ? -num : num;
                    i = j;
                }
            }
        }
        i++;
        if (s[i] == ',') {
            i++;
        }
    }
    return count;
}