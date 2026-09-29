#include <stdint.h>

void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0, neg = 0;
    unsigned int uval = (value == INT_MIN) ? (uint32_t)INT_MIN : (unsigned int)(value < 0 ? -value : value);
    
    if (value < 0 && value != INT_MIN) neg = 1;
    
    do {
        tmp[i++] = uval % 10 + '0';
    } while ((uval /= 10) > 0);
    
    if (neg) tmp[i++] = '-';
    
    int j = 0;
    do {
        buf[j++] = tmp[--i];
    } while (i > 0);
    
    buf[j] = '\0';
}