#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    unsigned int n = (unsigned int)value;
    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        is_negative = 1;
        n = (unsigned int)INT_MAX;
        n += 1u;
    } else if (value < 0) {
        is_negative = 1;
        n = (unsigned int)(-value);
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        char temp[12]; // Sufficient for 10 digits + sign + null
        int idx = 0;
        while (n > 0) {
            temp[idx++] = (char)('0' + (n % 10));
            n /= 10;
        }
        while (idx > 0) {
            *p++ = temp[--idx];
        }
    }

    if (is_negative) {
        *p++ = '-';
    }

    *p = '\0';
}