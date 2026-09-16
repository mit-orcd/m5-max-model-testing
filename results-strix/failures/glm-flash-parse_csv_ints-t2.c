#include <limits.h>
#include <stdlib.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    long val = 0;
    char sign = 1;
    char c;

    while (*s != '\0' && count < max) {
        c = *s++;
        if (c == '-') {
            sign = -1;
        } else if (c == ',') {
            if (sign == 1 && val > INT_MAX / 10) {
                return count;
            }
            if (sign == -1 && val > (unsigned long)INT_MAX + 1 / 10) {
                return count;
            }
            val *= 10;
            if (sign == 1 && val > INT_MAX - (c - '0')) {
                return count;
            }
            if (sign == -1 && val > (unsigned long)INT_MAX + 1 - (c - '0')) {
                return count;
            }
            val += (c - '0');
            out[count++] = (int)(sign * val);
            val = 0;
            sign = 1;
        } else if (c >= '0' && c <= '9') {
            if (sign == 1 && val > INT_MAX / 10) {
                return count;
            }
            if (sign == -1 && val > (unsigned long)INT_MAX + 1 / 10) {
                return count;
            }
            val *= 10;
            if (sign == 1 && val > INT_MAX - (c - '0')) {
                return count;
            }
            if (sign == -1 && val > (unsigned long)INT_MAX + 1 - (c - '0')) {
                return count;
            }
            val += (c - '0');
        }
    }
    if (count < max) {
        if (sign == 1 && val > INT_MAX / 10) {
            return count;
        }
        if (sign == -1 && val > (unsigned long)INT_MAX + 1 / 10) {
            return count;
        }
        val *= 10;
        if (sign == 1 && val > INT_MAX - (c - '0')) {
            return count;
        }
        if (sign == -1 && val > (unsigned long)INT_MAX + 1 - (c - '0')) {
            return count;
        }
        out[count++] = (int)(sign * val);
    }
    return count;
}