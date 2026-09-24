#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    char *start = buf;
    int neg = 0;
    if (value < 0) {
        neg = 1;
        value = -(value + 1);
    }
    int digits = 1;
    int tmp = value;
    while (tmp >= 10) {
        digits++;
        tmp /= 10;
    }
    if (neg) digits++;
    buf += digits;
    *buf = '\0';
    buf--;
    while (value > 0) {
        *buf-- = '0' + (value % 10);
        value /= 10;
    }
    if (neg) {
        *buf = '-';
    }
}