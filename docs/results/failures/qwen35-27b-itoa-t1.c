#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char *p = buf;
    char *end = buf;

    if (value < 0) {
        *p++ = '-';
        end = p;
    }

    int v = value;
    if (v < 0) {
        // Handle INT_MIN specifically to avoid overflow when negating
        // INT_MIN is -2147483648
        if (v == INT_MIN) {
            *p++ = '2';
            v = -2147483647; // Now positive range
        } else {
            v = -v;
        }
    }

    char *temp = p;

    // Extract digits in reverse order
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v > 0);
    *p = '\0';

    // Reverse the digits part (excluding sign if present)
    char *start = end;
    while (start < p - 1) {
        char tmp = *start;
        *start = *(p - 1);
        *(p - 1) = tmp;
        start++;
        p--;
    }
}