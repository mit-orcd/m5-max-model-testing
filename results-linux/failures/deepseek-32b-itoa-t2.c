#include <string.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        *buf = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        if (value == INT_MIN) {
            const char *s = "-2147483648";
            size_t len = strlen(s);
            memcpy(buf, s, len);
            buf[len] = '\0';
            return;
        } else {
            *buf = '-';
            buf++;
            value = -value;
        }
    }

    char *ptr = buf;
    int n = value;

    do {
        *ptr = (n % 10) + '0';
        ptr++;
        n /= 10;
    } while (n != 0);

    size_t len = ptr - buf;
    ptr--;

    for (size_t i = 0; i < len / 2; i++) {
        char temp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = temp;
    }

    *ptr = '\0';
}