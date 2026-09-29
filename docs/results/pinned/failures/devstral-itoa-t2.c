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
            value = 746364826;  // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 364826482;  // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 64826482;   // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 4826482;    // (INT_MAX + 1) % 10
            *buf++ = '4';
            value = 826482;     // (INT_MAX + 1) / 10
            *buf++ = '8';
            value = 26482;      // (INT_MAX + 1) % 10
            *buf++ = '2';
            value = 2648;       // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 648;        // (INT_MAX + 1) % 10
            *buf++ = '6';
            value = 64;         // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 4;          // (INT_MAX + 1) % 10
            *buf++ = '4';
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