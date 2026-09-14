#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int sign = 1;
    int val = 0;
    int in_number = 0;
    const char *p = s;

    while (*p) {
        if (isdigit((unsigned char)*p)) {
            if (!in_number) {
                in_number = 1;
                val = 0;
                sign = 1;
            }
            val = val * 10 + (*p - '0');
            p++;
        } else if (*p == '-' && !in_number) {
            sign = -1;
            in_number = 1;
            p++;
        } else if (*p == ',' || *p == '\0') {
            if (in_number) {
                if (count < max) {
                    out[count] = val * sign;
                    count++;
                }
                in_number = 0;
            }
            if (*p == '\0') break;
            p++;
        } else {
            p++;
        }
    }

    return count;
}