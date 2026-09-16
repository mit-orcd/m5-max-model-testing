#include <stdint.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0;
    int sign = value;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        // Handle INT_MIN specifically to avoid overflow when negating
        // INT_MIN is -2147483648
        if (value == INT32_MIN) {
            const char *s = "-2147483648";
            while (*s) {
                *buf++ = *s++;
            }
            *buf = '\0';
            return;
        }
        value = -value;
    }

    while (value > 0) {
        tmp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (sign < 0) {
        buf[i++] = '-';
    }

    while (i > 0) {
        *buf++ = tmp[--i];
    }
    *buf = '\0';
}