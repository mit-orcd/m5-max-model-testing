#include <limits.h>
#include <stddef.h>
#include <stdint.h>
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
    } while ((value /= 10) != 0);

    if (neg)
        buf[i++] = '-';

    buf[i] = '\0';
    reverse(buf, i);
}

void reverse(char *str, size_t len) {
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}