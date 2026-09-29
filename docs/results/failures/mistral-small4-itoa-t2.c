#include <stdbool.h>

void itoa(int value, char *buf) {
    if (buf == NULL) return;

    unsigned int uvalue = value;
    bool is_negative = false;

    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            uvalue = (unsigned int)(INT_MAX) + 1;
        } else {
            uvalue = (unsigned int)(-value);
        }
    }

    char *p = buf;
    do {
        *p++ = '0' + (uvalue % 10);
        uvalue /= 10;
    } while (uvalue > 0);

    if (is_negative) {
        *p++ = '-';
    }

    *p = '\0';

    char *start = buf;
    char *end = p - 1;

    if (is_negative) {
        end--;
    }

    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }

    if (is_negative && buf[0] == '0') {
        buf[0] = '-';
    }
}