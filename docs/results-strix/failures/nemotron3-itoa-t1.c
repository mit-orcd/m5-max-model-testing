#include <limits.h>

void itoa(int value, char *buf) {
    if (value == INT_MIN) {
        char *p = buf;
        *p++ = '-';
        int v = -(value + 1);
        if (v == 0) {
            *p++ = '2';
            *p++ = '1';
            *p++ = '4';
            *p++ = '7';
            *p++ = '4';
            *p++ = '8';
            *p++ = '3';
            *p++ = '6';
            *p++ = '4';
            *p++ = '8';
        } else {
            char temp[11];
            int i = 0;
            do {
                temp[i++] = '0' + (v % 10);
                v /= 10;
            } while (v > 0);
            while (i > 0) *p++ = temp[--i];
        }
        *p = '\0';
        return;
    }

    int negative = value < 0;
    if (negative) value = -value;

    char *p = buf;
    if (value == 0) {
        *p++ = '0';
    } else {
        char temp[12];
        int i = 0;
        while (value > 0) {
            temp[i++] = '0' + (value % 10);
            value /= 10;
        }
        while (i > 0) *p++ = temp[--i];
    }

    if (negative) *p++ = '-';
    *p = '\0';

    // Reverse the string since we built it backwards
    int len = 0;
    while (buf[len]) len++;
    for (int i = 0; i < len / 2; i++) {
        char t = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = t;
    }
}