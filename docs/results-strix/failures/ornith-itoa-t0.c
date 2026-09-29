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
            tmp[i++] = (char)('0' + (uval % 10u));
            uval /= 10u;
        }
    }

    if (is_neg) {
        buf[0] = '-';
        for (int j = 0; j < i; j++) {
            buf[j + 1] = tmp[j];
        }
        buf[i + 1] = '\0';
    } else {
        for (int j = 0; j < i; j++) {
            buf[j] = tmp[j];
        }
        buf[i] = '\0';
    }
}