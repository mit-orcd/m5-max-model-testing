#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    unsigned int n = (unsigned int)value;
    int len = 0;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN specially to avoid overflow when negating
        buf[0] = '-';
        buf[1] = '0';
        buf[2] = '1';
        buf[3] = '4';
        buf[4] = '7';
        buf[5] = '4';
        buf[6] = '8';
        buf[7] = '3';
        buf[8] = '6';
        buf[9] = '4';
        buf[10] = '8';
        buf[11] = '4';
        buf[12] = '1';
        buf[13] = '6';
        buf[14] = '\0';
        return;
    }

    if (value < 0) {
        buf[len++] = '-';
        n = (unsigned int)(-(value + 1)) + 1; // Safe negation
    } else if (value == 0) {
        buf[len++] = '0';
    } else {
        while (n != 0) {
            buf[len++] = (char)('0' + (n % 10));
            n /= 10;
        }
    }

    if (value == 0) {
        // Already handled, but ensure logic holds if entered else branch
    } else {
        // Reverse the string
        int start = 0;
        int end = len - 1;
        while (start < end) {
            char temp = buf[start];
            buf[start] = buf[end];
            buf[end] = temp;
            start++;
            end--;
        }
    }

    buf[len] = '\0';
}