#include <limits.h>

void itoa(int value, char *buf) {
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            // Handle INT_MIN specially to avoid overflow on negation
            value = -value - 1; // Now value is 2147483647
            buf[i++] = '2';
            value -= 2;         // Adjust to 2147483646
        } else {
            value = -value;
        }
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        start++;
        end--;
    }
}