#include <limits.h>
#include <string.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    int negative = value < 0;
    int i = 0;
    
    if (negative) {
        value = -value;
    }
    
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (negative) {
        buf[i] = '-';
        i++;
    }
    
    buf[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; i++, j++) {
        char temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
}