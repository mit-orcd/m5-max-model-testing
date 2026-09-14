#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        /* Handle INT_MIN specially to avoid overflow when negating */
        if (value == INT_MIN) {
            /* Convert to string manually for INT_MIN */
            /* -2147483648 */
            p += 10;
            *--p = '8';
            *--p = '4';
            *--p = '6';
            *--p = '3';
            *--p = '8';
            *--p = '4';
            *--p = '7';
            *--p = '4';
            *--p = '1';
            *--p = '2';
            *--p = '-';
            *p = '\0';
            return;
        }
        value = -value;
    }

    while (value > 0) {
        *p++ = (char)('0' + (value % 10));
        value /= 10;
    }

    if (is_negative) {
        *p++ = '-';
    }

    /* Reverse the string */
    char *start = buf;
    char *end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }

    *p = '\0';
}