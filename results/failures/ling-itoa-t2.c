#include <stdint.h>

void itoa(int value, char *buf) {
    uint32_t u;
    char *start = buf;
    char *end;
    char tmp;

    if (value == INT32_MIN) {
        *buf++ = '-';
        *buf++ = '2';
        *buf++ = '1';
        *buf++ = '4';
        *buf++ = '7';
        *buf++ = '4';
        *buf++ = '8';
        *buf++ = '3';
        *buf++ = '6';
        *buf++ = '4';
        *buf++ = '8';
        *buf = '\0';
        return;
    }

    if (value < 0) {
        *buf++ = '-';
        u = (uint32_t)-value;
    } else {
        u = (uint32_t)value;
    }

    end = buf;
    do {
        *end++ = '0' + (u % 10);
        u /= 10;
    } while (u > 0);

    while (end > buf) {
        *start++ = *--end;
    }
    *start = '\0';
}