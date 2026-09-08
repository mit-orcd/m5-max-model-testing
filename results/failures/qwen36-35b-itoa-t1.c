#include <limits.h>
#include <stdint.h>

void itoa(int value, char *buf) {
    uint32_t uvalue = (uint32_t)value;
    char *p = buf;
    char *start = buf;
    
    /* Handle the special case of INT_MIN */
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    /* Handle negative numbers */
    if (value < 0) {
        *p++ = '-';
        /* Use unsigned arithmetic to avoid undefined behavior with ~INT_MIN + 1 */
        uvalue = (uint32_t)(0 - value);
    }

    /* Convert digits */
    uint32_t temp = uvalue;
    while (temp > 0) {
        *p++ = (char)('0' + (temp % 10));
        temp /= 10;
    }

    /* Reverse the string */
    p--;
    while (start < p) {
        char tmp = *start;
        *start = *p;
        *p = tmp;
        start++;
        p--;
    }
    *p = '\0';
}