#include <stdint.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0, neg = 0;
    unsigned int uval;

    if (value < 0) {
        neg = 1;
        if (value == INT_MIN) {
            uval = (unsigned int)(INT_MIN);
        } else {
            uval = (unsigned int)(-value);
        }
    } else {
        uval = (unsigned int)value;
    }

    do {
        tmp[i++] = "0123456789"[uval % 10];
        uval /= 10;
    } while (uval);

    if (neg) {
        tmp[i++] = '-';
    }

    tmp[i] = '\0';

    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - j - 1];
    }
    buf[i] = '\0';
}