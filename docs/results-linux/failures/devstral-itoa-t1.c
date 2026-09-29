#include <stdint.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        *buf = '0';
        *(buf + 1) = '\0';
        return;
    }

    int is_negative = 0;
    if (value < 0) {
        if (value == INT_MIN) {
            // Handle INT_MIN separately to avoid overflow
            const char *s = "-2147483648";
            int i = 0;
            while (s[i] != '\0') {
                buf[i] = s[i];
                i++;
            }
            buf[i] = '\0';
            return;
        }
        is_negative = 1;
        value = -value;
    }

    char temp[12]; // Enough for 32-bit int
    int i = 0;
    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        temp[i++] = '-';
    }

    // Reverse the digits
    int j = 0;
    while (i > 0) {
        i--;
        buf[j++] = temp[i];
    }
    buf[j] = '\0';
}