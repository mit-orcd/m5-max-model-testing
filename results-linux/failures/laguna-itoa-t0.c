#include <limits.h>

void itoa(int value, char *buf) {
    char temp[12];
    int i = 0;
    unsigned int uvalue;
    int negative = 0;
    
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    if (value < 0) {
        negative = 1;
        uvalue = (unsigned int)(-(value + 1)) + 1;
    } else {
        uvalue = (unsigned int)value;
    }
    
    while (uvalue > 0) {
        temp[i++] = '0' + (uvalue % 10);
        uvalue /= 10;
    }
    
    if (negative) {
        buf[i++] = '-';
    }
    
    int j;
    for (j = i - 1; j >= 0; j--) {
        buf[i++] = temp[j];
    }
    buf[i] = '\0';
}