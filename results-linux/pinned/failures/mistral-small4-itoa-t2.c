#include <stdbool.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (buf == NULL) return;

    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            value = -(value / 10);
            buf[0] = '-';
            buf[1] = '2';
            buf += 2;
        } else {
            value = -value;
        }
    }

    char *start = buf;
    do {
        *buf++ = '0' - value % 10;
        value /= 10;
    } while (value != 0);

    if (is_negative) {
        *buf++ = '-';
    }

    *buf = '\0';

    char *end = buf - 1;
    char *p = start;
    while (p < end) {
        char tmp = *p;
        *p++ = *end;
        *end-- = tmp;
    }
}