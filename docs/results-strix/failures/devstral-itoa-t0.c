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
            value = 463648286;  // (INT_MAX + 1) % 10
            *buf++ = '4';
            value = 746340269;  // (INT_MAX + 1) / 100
            *buf++ = '6';
            value = 32768;      // (INT_MAX + 1) % 100
            *buf++ = '3';
            value = 65536;      // (INT_MAX + 1) / 1000
            *buf++ = '6';
            value = 48286;      // (INT_MAX + 1) % 1000
            *buf++ = '4';
            value = 6553;       // (INT_MAX + 1) / 10000
            *buf++ = '8';
            value = 286;        // (INT_MAX + 1) % 10000
            *buf++ = '2';
            value = 65;         // (INT_MAX + 1) / 100000
            *buf++ = '6';
            value = 5;          // (INT_MAX + 1) % 100000
            *buf++ = '5';
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