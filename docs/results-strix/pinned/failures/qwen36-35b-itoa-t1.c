#include <limits.h>
#include <string.h>

void itoa(int value, char *buf) {
    char temp[12];
    int i = 0;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        // INT_MIN is -2147483648
        const char *s = "-2147483648";
        while (*s) {
            buf[i++] = *s++;
        }
        buf[i] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0) {
            temp[i++] = (value % 10) + '0';
            value /= 10;
        }
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        start++;
        end--;
    }
}