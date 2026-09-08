#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        strcpy(buf, "0");
        return;
    }

    int neg = value < 0;
    if (neg)
        value = -value;

    int i = 0;
    do {
        buf[i++] = '0' + value % 10;
        value /= 10;
    } while (value != 0);

    if (neg)
        buf[i++] = '-';

    buf[i] = '\0';
    reverse(buf);
}

void reverse(char *str) {
    int i, j;
    char tmp;
    for (i = 0, j = strlen(str) - 1; i < j; i++, j--) {
        tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}