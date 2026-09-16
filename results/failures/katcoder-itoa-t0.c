#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0;
    int neg = 0;

    if (value == INT_MIN) {
        neg = 1;
        value = INT_MAX;
    } else if (value < 0) {
        neg = 1;
        value = -value;
    }

    do {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (neg) {
        tmp[i++] = '-';
    }

    int j;
    for (j = 0; j < i; j++) {
        *buf++ = tmp[i - 1 - j];
    }
    *buf = '\0';
}