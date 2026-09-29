#include <stdint.h>

void itoa(int value, char *buf) {
    uint32_t u;
    char *start = buf;
    char *end;
    char tmp;

    if (value < 0) {
        *buf++ = '-';
        u = (uint32_t)-(int64_t)value;
    } else {
        u = (uint32_t)value;
    }

    end = buf;
    do {
        *end++ = '0' + (u % 10);
        u /= 10;
    } while (u > 0);

    *end = '\0';

    end--;
    while (start < end) {
        tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}