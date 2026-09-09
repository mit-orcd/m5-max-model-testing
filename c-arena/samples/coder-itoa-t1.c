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
    int temp = value;
    
    if (negative) {
        temp = -value;
    }
    
    while (temp > 0) {
        buf[i++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    if (negative) {
        buf[i] = '-';
        i++;
    }
    
    buf[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; i++, j++) {
        char c = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = c;
    }
}