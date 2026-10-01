#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int negative = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value == INT_MIN) {
        *p++ = '-';
        unsigned int u = (unsigned int)(-(value + 1)) + 1u;
        negative = 1;
        value = 0;
    } else if (value < 0) {
        *p++ = '-';
        u = (unsigned int)(-value);
        negative = 1;
        value = 0;
    }

    if (value != 0) {
        unsigned int u = (unsigned int)value;
        char tmp[12];
        char *t = tmp;
        while (u > 0) {
            *t++ = (char)('0' + (u % 10));
            u /= 10;
        }
        while (t > tmp) {
            *p++ = *--t;
        }
    }

    *p = '\0';
}