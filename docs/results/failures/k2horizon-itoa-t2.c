#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = value < 0;

    if (negative) {
        *p++ = '-';
        if (value == INT_MIN) {
            unsigned int u = (unsigned int)(value + 1) + (unsigned int)1;
            value = (int)(u - 1);
        } else {
            value = -value;
        }
    }

    char tmp[11];
    int i = 0;
    int digit;

    do {
        digit = value % 10;
        tmp[i++] = (char)('0' + digit);
        value /= 10;
    } while (value != 0);

    while (i > 0) {
        *p++ = tmp[--i];
    }
    *p = '\0';
}