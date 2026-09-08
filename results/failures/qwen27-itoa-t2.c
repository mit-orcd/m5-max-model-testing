#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    if (value < 0) {
        *p++ = '-';
        /* Handle INT_MIN by working in unsigned */
        unsigned int uval;
        if (value == INT_MIN) {
            uval = 2147483648U; /* -INT_MIN as unsigned */
        } else {
            uval = (unsigned int)(-value);
        }
        char tmp[11];
        char *t = tmp;
        do {
            *t++ = (char)('0' + (uval % 10));
            uval /= 10;
        } while (uval != 0);
        *t = '\0';
        /* Reverse tmp into buf */
        while (*t != '\0') {
            *p++ = *t++;
        }
        *p = '\0';
    } else {
        unsigned int uval = (unsigned int)value;
        char tmp[11];
        char *t = tmp;
        do {
            *t++ = (char)('0' + (uval % 10));
            uval /= 10;
        } while (uval != 0);
        *t = '\0';
        while (*t != '\0') {
            *p++ = *t++;
        }
        *p = '\0';
    }
}