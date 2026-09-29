#include <limits.h>

void itoa(int value, char *buf) {
    char *ptr = buf;
    unsigned int uv = value;

    if (value < 0) {
        uv = -value;
        *ptr++ = '-';
    }

    // Special case for 0
    if (uv == 0) {
        *ptr++ = '0';
    } else {
        // Calculate number of digits
        int num_digits = 0;
        unsigned int temp = uv;
        while (temp != 0) {
            temp /= 10;
            num_digits++;
        }

        // Calculate starting pointer
        ptr += num_digits - 1;

        // Store digits
        while (uv != 0) {
            *ptr-- = '0' + (uv % 10);
            uv /= 10;
        }
    }

    *ptr = '\0';
}