#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    int sign = 1;
    long val = 0;
    int in_number = 0;

    while (*p && count < max) {
        if (isdigit((unsigned char)*p)) {
            if (!in_number) {
                in_number = 1;
                sign = 1;
                val = 0;
            }
            val = val * 10 + (*p - '0');
        } else if (*p == '-' && !in_number) {
            sign = -1;
            in_number = 1;
        } else if (*p == ',' || *p == '\0') {
            if (in_number) {
                out[count++] = (int)(val * sign);
                in_number = 0;
            }
            if (*p == '\0') break;
        }
        p++;
    }

    return count;
}