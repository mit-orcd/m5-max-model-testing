#include <limits.h>
#include <stdint.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    int negative = value < 0;
    int i = 0;
    char temp[32]; // Enough for any 32-bit integer
    
    if (negative) {
        value = -value;
    }
    
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (negative) {
        buf[0] = '-';
        i = 1;
    } else {
        i = 0;
    }
    
    int j = 0;
    while (i > 0) {
        buf[j++] = temp[--i];
    }
    
    buf[j] = '\0';
}