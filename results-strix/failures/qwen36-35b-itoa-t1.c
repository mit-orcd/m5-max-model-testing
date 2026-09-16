#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    char temp[12];
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        /* Handle INT_MIN carefully to avoid overflow when negating */
        if (value == INT_MIN) {
            /* INT_MIN is -2147483648 */
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
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    while (i > 0) {
        *buf++ = temp[--i];
    }
    *buf = '\0';
}