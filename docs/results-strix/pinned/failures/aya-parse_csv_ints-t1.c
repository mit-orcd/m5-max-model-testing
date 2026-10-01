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
            out[count] = neg * (s[i] - '0');
            neg = 0;
            i++;
        } else if (s[i] == ',' && s[i + 1] != ' ') {
            i++;
            count++;
        } else if (s[i] == ' ' && s[i + 1] != ',') {
            i++;
        } else {
            fprintf(stderr, "Invalid input at position %d\n", i);
            return -1;
        }
    }

    return count;
}