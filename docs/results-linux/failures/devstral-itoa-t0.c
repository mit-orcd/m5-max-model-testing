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
            value = 746158176;  // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 322122547;  // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 644245094;  // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 231700643;  // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 463401287;  // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 143165576;  // (INT_MAX + 1) % 10
            *buf++ = '6';
            value = 286331153;  // (INT_MAX + 1) / 10
            *buf++ = '3';
            value = 143165576;  // (INT_MAX + 1) % 10
            *buf++ = '6';
            value = 14316557;   // (INT_MAX + 1) % 10
            *buf++ = '5';
            value = 1431655;    // (INT_MAX + 1) / 10
            *buf++ = '5';
            value = 715827;     // (INT_MAX + 1) % 10
            *buf++ = '7';
            value = 71582;      // (INT_MAX + 1) / 10
            *buf++ = '2';
            value = 7158;       // (INT_MAX + 1) % 10
            *buf++ = '8';
            value = 715;        // (INT_MAX + 1) / 10
            *buf++ = '5';
            value = 71;         // (INT_MAX + 1) % 10
            *buf++ = '1';
            value = 7;          // (INT_MAX + 1) / 10
            *buf++ = '7';
            *buf++ = '0';
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