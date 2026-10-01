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
            value = 746340269;  // (INT_MAX + 1) / 10
            *buf++ = '6';
            value = 322384093;  // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 644768186;  // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 188495440;  // (INT_MAX + 1) % 10
            *buf++ = '8';
            value = 37699088;   // (INT_MAX + 1) / 10
            *buf++ = '2';
            value = 18849544;   // (INT_MAX + 1) % 10
            *buf++ = '8';
            value = 1884954;    // (INT_MAX + 1) / 10
            *buf++ = '4';
            value = 942477;     // (INT_MAX + 1) % 10
            *buf++ = '7';
            value = 94247;      // (INT_MAX + 1) / 10
            *buf++ = '7';
            value = 47123;      // (INT_MAX + 1) % 10
            *buf++ = '3';
            value = 4712;       // (INT_MAX + 1) / 10
            *buf++ = '2';
            value = 359;        // (INT_MAX + 1) % 10
            *buf++ = '5';
            value = 35;         // (INT_MAX + 1) / 10
            *buf++ = '9';
            value = 5;          // (INT_MAX + 1) % 10
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