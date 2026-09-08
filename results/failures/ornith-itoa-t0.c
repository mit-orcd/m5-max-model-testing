#include <limits.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0, is_neg = 0;
    unsigned int uval;

    if (value < 0) {
        is_neg = 1;
        uval = (unsigned int)(-(value + 1)) + 1u;
    } else {
        uval = (unsigned int)value;
    }

    if (uval == 0) {
        tmp[i++] = '0';
    } else {
        while (uval > 0) {
            tmp[i++] = (char)('0' + (uval % 10));
            uval /= 10;
        }
    }

    if (is_neg) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    for (int j = 0, k = (is_neg ? i - 1 : i - 1); j < k; j++, k--) {
        char c = buf[j];
        buf[j] = buf[k];
        buf[k] = c;
    }
}