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
            value = 1431655765; // (INT_MAX + 1) / 10
            *buf++ = '1';
            value = 463648264;  // (INT_MAX + 1) % 10
            *buf++ = '4';
            value = 746348264;  // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 348264;     // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 48264;      // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 8264;       // (INT_MAX + 1) % 10
            *buf++ = '8';
            value = 264;        // (INT_MAX + 1) / 10
            *buf++ = '2';
            value = 64;         // (INT_MAX + 1) % 10
            *buf++ = '6';
            value = 6;          // (INT_MAX + 1) / 10
            *buf++ = '4';
            *buf++ = '7';
            *buf = '\0';
            return;
        }
        value = -value;
    }

    char temp[12];
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