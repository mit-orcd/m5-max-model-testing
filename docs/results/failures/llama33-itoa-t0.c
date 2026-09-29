#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    int i = 0;
    int sign = 1;
    if (value < 0) {
        sign = -1;
        value = -value;
    }

    if (value == INT_MIN) {
        value = INT_MAX + 1;
        buf[i++] = '-';
    } else if (sign == -1) {
        buf[i++] = '-';
    }

    while (value > 0) {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (i == 0) {
        buf[i++] = '0';
    }

    buf[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}