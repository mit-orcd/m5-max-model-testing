#include <stdbool.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (buf == NULL) return;

    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            value = -(INT_MIN / 10);
            buf[0] = '-';
            buf[1] = '2';
            buf[2] = '1';
            buf += 3;
        } else {
            value = -value;
        }
    }

    char *start = buf;
    do {
        *buf++ = (char)('0' + (value % 10));
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        *buf++ = '-';
    }

    *buf = '\0';

    char *end = buf - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}