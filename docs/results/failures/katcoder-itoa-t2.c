#include <limits.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0;
    int neg = 0;

    if (value < 0 && value != INT_MIN) {
        neg = 1;
        value = -value;
    } else if (value == INT_MIN) {
        neg = 1;
        value = -(value + 1);
        i++;
        tmp[0] = '2';
    }

    do {
        tmp[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (neg) {
        tmp[i++] = '-';
    }

    int j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}