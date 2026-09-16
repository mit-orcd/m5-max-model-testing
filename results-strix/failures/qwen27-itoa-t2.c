#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        buf[0] = '-';
        // Handle INT_MIN specially since -INT_MIN overflows
        if (value == INT_MIN) {
            const char *neg_min = "2147483648";
            size_t i = 1;
            for (const char *p = neg_min; *p; ++p, ++i) {
                buf[i] = *p;
            }
            buf[i] = '\0';
            return;
        }
        value = -value;
    }

    char *end = buf;
    while (value > 0) {
        *end++ = (char)('0' + (value % 10));
        value /= 10;
    }
    *end = '\0';

    // Reverse the digits
    char *start = buf;
    if (buf[0] == '-') {
        start++;
    }
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}