#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf)
{
    int negative = 0;
    unsigned int n;
    char *p = buf;
    char temp;

    if (value < 0) {
        negative = 1;
        /* Handle INT_MIN without undefined behavior */
        n = (unsigned int)(-(long)value);
    } else {
        n = (unsigned int)value;
    }

    /* Special case for zero */
    if (n == 0) {
        *p++ = '0';
    } else {
        /* Convert digits in reverse order */
        while (n > 0) {
            *p++ = (char)('0' + (n % 10));
            n /= 10;
        }
    }

    /* Add negative sign if needed */
    if (negative) {
        *p++ = '-';
    }

    *p = '\0';

    /* Reverse the string in-place */
    p = buf;
    if (negative) {
        /* Skip the negative sign for reversal */
        char *end = buf + (p - buf);
        while (*p) p++;
        p--; /* Point to last character before '\0' */
        if (negative) {
            /* Move past the '-' sign */
            char *start = buf + 1;
            while (start < p) {
                temp = *start;
                *start = *p;
                *p = temp;
                start++;
                p--;
            }
        } else {
            char *start = buf;
            while (start < p) {
                temp = *start;
                *start = *p;
                *p = temp;
                start++;
                p--;
            }
        }
    } else {
        char *start = buf;
        char *end = buf;
        while (*end) end++;
        end--;
        while (start < end) {
            temp = *start;
            *start = *end;
            *end = temp;
            start++;
            end--;
        }
    }
}