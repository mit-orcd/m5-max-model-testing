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
    } while (value);

    if (neg)
        buf[i++] = '-';

    buf[i] = '\0';
    reverse(buf);
}

void reverse(char *str) {
    char *end = strchr(str, '\0');
    for (char *p = str, *q = end - 1; p < q; p++, q--) {
        char tmp = *p;
        *p = *q;
        *q = tmp;
    }
}