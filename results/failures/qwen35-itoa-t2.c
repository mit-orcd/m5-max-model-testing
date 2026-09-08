#include <limits.h>

void itoa(int value, char *buf) {
    int i = 0;
    unsigned int uval;
    int is_neg = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        // INT_MIN is -2147483648
        buf[i++] = '-';
        uval = (unsigned int)INT_MAX + 1U; // Which is 2147483648
    } else if (value < 0) {
        buf[i++] = '-';
        uval = (unsigned int)(-value);
    } else {
        uval = (unsigned int)value;
    }

    if (uval == 0) {
        buf[i++] = '0';
    } else {
        while (uval > 0) {
            buf[i++] = '0' + (uval % 10);
            uval /= 10;
        }
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