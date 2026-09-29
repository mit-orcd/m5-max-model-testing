#include <limits.h>

void itoa(int value, char *buf)
{
    unsigned int magnitude;
    char temporary[1 + sizeof(unsigned int) * CHAR_BIT / 3];
    size_t length = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        magnitude = 0u - (unsigned int)value;
        buf[length++] = '-';
    } else {
        magnitude = (unsigned int)value;
    }

    while (magnitude > 0u) {
        temporary[length++] = (char)('0' + (magnitude % 10u));
        magnitude /= 10u;
    }

    while (length > 0u) {
        buf[--length] = temporary[length];
    }
    buf[0] = '\0';
}