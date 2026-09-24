#include <stdbool.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (buf == NULL) return;
    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            value = -(value + 1);
            *buf++ = '2';
            value = (INT_MIN / 10) - (value / 10);
        } else {
            value = -value;
        }
    }
    char *start = buf;
    do {
        *buf++ = '0' + (value % 10);
        value /= 10;
    } while (value != 0);
    if (is_negative) {
        *buf++ = '-';
    }
    *buf = '\0';
    char *end = buf - 1;
    if (*end == '-') end--;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
    if (is_negative && (buf - start) > 1) {
        if (*(start + 1) == '1') {
            *(start + 1) = '8';
            start += 2;
            while (*start) {
                *start++ = '0';
            }
        } else {
            *(start + 1)--;
            start += 2;
            while (*start) {
                *start++ = '0';
            }
        }
    }
}