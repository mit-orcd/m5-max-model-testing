#include <stdlib.h>

void itoa(int value, char *buf) {
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            value = -(value / 10);
            buf[i++] = '0' - (value % 10);
        } else {
            value = -value;
        }
    }

    while (value != 0) {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    for (int j = 0, k = i - 1; j < k / 2; j++, k--) {
        char temp = buf[j];
        buf[j] = buf[k - j - 1];
        buf[k - j - 1] = temp;
    }
}