#include <stdio.h>
#include <stdint.h>
#include <string.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char *p = buf;
    if (value < 0) {
        *p++ = '-';
        value = -value;
    }

    int len = 0;
    int value_copy = value;
    while (value_copy != 0) {
        value_copy /= 10;
        len++;
    }

    if (len == 0) {
        *p++ = '0';
    } else {
        for (int i = len - 1; i >= 0; i--) {
            *p++ = (value / (int)pow(10, i)) % 10 + '0';
        }
    }
    *p = '\0';
}