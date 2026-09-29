#include <limits.h>
#include <stdlib.h>

void itoa(int value, char *buf) {
    int i = 0;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        buf[i++] = '-';
        value = 2147483648; // -INT_MIN
    } else if (value < 0) {
        buf[i++] = '-';
        value = -value;
    }

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            buf[i++] = (value % 10) + '0';
            value /= 10;
        }
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