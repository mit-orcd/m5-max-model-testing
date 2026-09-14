#include <inttypes.h>
#include <stdio.h>
#include <string.h>

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

void reverse(char *str, size_t len) {
    char *end = str + len - 1;
    while (str < end) {
        char tmp = *str;
        *str = *end;
        *end = tmp;
        str++;
        end--;
    }
}