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
    int start = 0;
    
    if (negative) {
        value = -value;
    }
    
    while (value > 0) {
        buf[i] = '0' + (value % 10);
        value /= 10;
        i++;
    }
    
    if (negative) {
        buf[i] = '-';
        start = 1;
    }
    
    int j = 0;
    for (int k = i - 1; k >= 0; k--) {
        buf[j] = buf[k];
        j++;
    }
    
    buf[i] = '\0';
}