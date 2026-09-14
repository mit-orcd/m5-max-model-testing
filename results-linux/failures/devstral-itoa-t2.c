#include <limits.h>
#include <string.h>

void itoa(int value, char *buf) {
    char temp[sizeof(int) * 3 + 1]; // Enough for INT_MIN (-2147483648)
    int i = 0;
    unsigned int uvalue = value;

    if (value < 0) {
        if (value == INT_MIN) {
            uvalue = (unsigned int)INT_MIN;
            temp[i++] = '-';
        } else {
            *buf++ = '-';
            uvalue = -value;
        }
    }

    do {
        temp[i++] = '0' + uvalue % 10;
        uvalue /= 10;
    } while (uvalue > 0);

    if (value < 0 && value != INT_MIN) {
        buf--;
    }

    while (i-- > 0) {
        *buf++ = temp[i];
    }

    *buf = '\0';
}