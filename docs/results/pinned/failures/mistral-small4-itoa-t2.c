#include <stdbool.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (buf == NULL) return;

    bool is_negative = false;
    if (value < 0) {
        is_negative = true;
        if (value == INT_MIN) {
            value = -(value / 10);
            buf[0] = '-';
            buf[1] = '2';
            buf += 2;
        } else {
            value = -value;
        }
    }

    char *start = buf;
    do {
        *buf++ = '0' - value % 10;
        value /= 10;
    } while (value != 0);

    if (is_negative) {
        *buf++ = '-';
    }

    int len = buf - start;
    for (int i = 0, j = len - 1; i < j; i++, j--) {
        char tmp = start[i];
        start[i] = start[j];
        start[j] = tmp;
    }

    *buf = '\0';
}