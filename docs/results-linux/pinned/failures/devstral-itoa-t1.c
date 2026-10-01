#include <limits.h>
#include <stdbool.h>

void itoa(int value, char *buf) {
    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            // Handle INT_MIN separately to avoid overflow
            *buf++ = '-';
            value = -(value + 1);
            *buf++ = '2';
            value = 1431655766; // INT_MIN + 1 = 2147483647, so 2147483647 / 10 = 214748364
            *buf++ = '1';
            value = 43165576;
            *buf++ = '4';
            value = 7483647;
            *buf++ = '7';
            value = 483647;
            *buf++ = '4';
            value = 83647;
            *buf++ = '8';
            value = 3647;
            *buf++ = '3';
            value = 647;
            *buf++ = '6';
            value = 47;
            *buf++ = '4';
            value = 7;
            *buf++ = '7';
            *buf = '\0';
            return;
        }
        value = -value;
    }

    char temp[12]; // Enough for INT_MAX (10 digits) + NUL
    int i = 0;
    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        *buf++ = '-';
    }

    while (i > 0) {
        *buf++ = temp[--i];
    }
    *buf = '\0';
}