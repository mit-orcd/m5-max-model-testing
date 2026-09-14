#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        *p++ = '-';
        value = -(value + 1); // value becomes -(INT_MIN + 1) = INT_MAX
        // We will handle the last digit manually
        *p++ = '0' + (value % 10);
        value /= 10;
    } else if (value < 0) {
        *p++ = '-';
        value = -value;
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        while (value > 0) {
            *p++ = '0' + (value % 10);
            value /= 10;
        }
    }

    *p = '\0';

    // Reverse the string
    char *start = buf;
    char *end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}