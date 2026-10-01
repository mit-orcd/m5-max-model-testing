#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = value < 0;

    if (negative) {
        *p++ = '-';
        if (value == INT_MIN) {
            value = INT_MAX;
        } else {
            value = -value;
        }
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        char tmp[12];
        int i = 0;
        while (value > 0) {
            tmp[i++] = (char)('0' + (value % 10));
            value /= 10;
        }
        while (i > 0) {
            *p++ = tmp[--i];
        }
    }

    *p = '\0';
}