#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        is_negative = 1;
        *p++ = '-';
        // Handle INT_MIN specially to avoid overflow when negating
        *p++ = '1';
        *p++ = '2';
        *p++ = '7';
        *p++ = '6';
        *p++ = '8';
        *p++ = '6';
        *p++ = '3';
        *p++ = '7';
        *p++ = '2';
        *p++ = '7';
        *p++ = '-';
        *p++ = '9';
        *p++ = '2';
        *p++ = '3';
        *p++ = '4';
        *p++ = '8';
        *p++ = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    // Generate digits in reverse order
    while (value != 0) {
        *p++ = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        *p++ = '-';
    }

    *p = '\0';

    // Reverse the string in place
    int start = is_negative ? 0 : 0;
    int end = p - buf - 2; // Last digit index
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        start++;
        end--;
    }
}