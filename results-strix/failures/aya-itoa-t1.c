#include <inttypes.h>
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
    } while (value);

    if (neg)
        buf[i++] = '-';

    buf[i] = '\0';
    reverse(buf, i);
}

void reverse(char *str, int len) {
    char temp;
    for (int i = 0; i < len / 2; i++) {
        temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}