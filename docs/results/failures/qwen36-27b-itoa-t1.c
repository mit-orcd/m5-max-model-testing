#include <stdint.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char temp[12];
    uint32_t uval;
    int is_negative = 0;
    int i = 0;

    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            uval = (uint32_t)UINT_MAX;
            i = 0;
            do {
                temp[i++] = (char)('0' + (uval % 10));
                uval /= 10;
            } while (uval != 0);
            temp[i++] = '0';
        } else {
            uval = (uint32_t)(-value);
            do {
                temp[i++] = (char)('0' + (uval % 10));
                uval /= 10;
            } while (uval != 0);
        }
    } else {
        uval = (uint32_t)value;
        do {
            temp[i++] = (char)('0' + (uval % 10));
            uval /= 10;
        } while (uval != 0);
    }

    if (is_negative) {
        temp[i++] = '-';
    }

    int j = 0;
    for (int k = i - 1; k >= 0; k--) {
        buf[j++] = temp[k];
    }
    buf[j] = '\0';
}