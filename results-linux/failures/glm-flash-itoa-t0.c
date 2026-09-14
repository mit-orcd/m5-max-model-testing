#include <limits.h>
#include <stdint.h>

void itoa(int value, char *buf) {
    uint32_t n;
    int sign = 0;

    if (value < 0) {
        sign = 1;
        n = (uint32_t)(-(int64_t)value);
    } else {
        n = (uint32_t)value;
    }

    char *p = buf;
    if (sign) {
        *p++ = '-';
    }

    if (n == 0) {
        *p++ = '0';
    } else {
        // Calculate number of digits
        uint32_t tmp = n;
        int digits = 0;
        while (tmp > 0) {
            tmp /= 10;
            digits++;
        }

        // Fill buffer from end to start
        p += digits;
        *p = '\0';
        while (digits--) {
            *--p = '0' + (n % 10);
            n /= 10;
        }
    }
    *p = '\0';
}