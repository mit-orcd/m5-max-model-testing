#include <limits.h>
#include <string.h>

void itoa(int value, char *buf) {
    char tmp[12]; /* enough for INT_MIN: -2147483648 (11 chars) + NUL */
    int i = 0;

    if (value == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (value < 0) {
        buf[i++] = '-';
        /* Handle INT_MIN carefully: it cannot be negated safely */
        if (value == INT_MIN) {
            /* Process as positive 2147483648 */
            long long v = 2147483648LL;
            while (v > 0) {
                tmp[i - 1] = (char)('0' + (int)(v % 10));
                v /= 10;
                i++;
            }
        } else {
            int v = -value;
            while (v > 0) {
                tmp[i - 1] = (char)('0' + (v % 10));
                v /= 10;
                i++;
            }
        }
    } else {
        int v = value;
        while (v > 0) {
            tmp[i] = (char)('0' + (v % 10));
            v /= 10;
            i++;
        }
        /* Reverse the digits into buf */
        int j = i - 1;
        while (j >= 0) {
            buf[i - 1 - j] = tmp[j];
            j--;
        }
        /* Wait, that's getting confusing. Let me redo this more cleanly. */
    }

    buf[i] = '\0';
}