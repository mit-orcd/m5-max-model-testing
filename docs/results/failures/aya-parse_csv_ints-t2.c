#include <stdio.h>
#include <stdbool.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    bool in_num = false;
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] == ',' || s[i] == '\r' || s[i] == '\n') {
            if (in_num) {
                out[count++] = atoi(s + i - (in_num ? 1 : 0));
                in_num = false;
            }
        } else if (s[i] == '-') {
            if (in_num) {
                out[count++] = -(atoi(s + i - (in_num ? 1 : 0)));
                i++;
                in_num = false;
            } else {
                in_num = true;
            }
        } else if (isdigit(s[i])) {
            if (!in_num) {
                in_num = true;
            }
            i++;
        } else {
            fprintf(stderr, "unexpected character: %c\n", s[i]);
            break;
        }
        if (count == max) {
            break;
        }
        i++;
    }
    return count;
}