#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        strcpy(buf, "0");
        return;
    }

    if (value < 0) {
        value = -value;
        buf[0] = '-';
    }

    int i = 0;
    do {
        buf[i++] = '0' + value % 10;
        value /= 10;
    } while (value);

    strcpy(buf + i, INT_MIN_STR);
}