#include <stdio.h>
#include <limits.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0;
    int isNegative = 0;

    if (value == INT_MIN) {
        isNegative = 1;
        value = INT_MAX;
        value += 1;
    } else if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    do {
        tmp[i++] = (char)('0' + (value % 10));
        value /= 10;
    } while (value > 0);

    if (isNegative) {
        tmp[i++] = '-';
    }
    tmp[i] = '\0';

    int j;
    for (j = 0; j < i / 2; j++) {
        char c = tmp[j];
        tmp[j] = tmp[i - j - 1];
        tmp[i - j - 1] = c;
    }

    for (j = 0; j <= i; j++) {
        buf[j] = tmp[j];
    }
}