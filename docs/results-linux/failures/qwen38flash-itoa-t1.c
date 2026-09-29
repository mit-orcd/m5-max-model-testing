#include <limits.h>

void itoa(int value, char *buf) {
    if (buf == NULL) {
        return;
    }

    // Handle the special case of INT_MIN to avoid overflow when negating
    long num = (long)value;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Temporary buffer to hold digits in reverse order
    char temp[12]; // Max digits for long (typically 19, but int is max 11 chars including sign and null)
    int i = 0;

    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = (char)(num % 10) + '0';
            num /= 10;
        }
    }

    // Add sign if negative
    int j = 0;
    if (is_negative) {
        buf[j++] = '-';
    }

    // Copy digits in reverse order (from temp back to buf)
    while (i > 0) {
        buf[j++] = temp[--i];
    }

    // Null terminate
    buf[j] = '\0';
}