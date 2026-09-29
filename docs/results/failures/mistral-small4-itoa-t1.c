#include <stdbool.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            value = -(INT_MIN / 10);
            buf[0] = '-';
            buf[1] = '2';
            buf[2] = '1';
            buf[3] = '4';
            buf[4] = '7';
            buf[5] = '4';
            buf[6] = '8';
            buf[7] = '3';
            buf[8] = '6';
            buf[9] = '4';
            buf[10] = '\0';
            return;
        }
        value = -value;
    }

    char *ptr = buf;
    if (is_negative) {
        *ptr++ = '-';
    }

    char temp[12];
    char *temp_ptr = temp;
    do {
        *temp_ptr++ = (char)(value % 10 + '0');
        value /= 10;
    } while (value != 0);

    if (is_negative) {
        temp_ptr--;
    }

    const char *end = temp_ptr;
    do {
        *ptr++ = *--temp_ptr;
    } while (temp_ptr != end);

    *ptr = '\0';
}