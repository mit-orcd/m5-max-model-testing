#include <stdint.h>
#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    int negative = 0;
    if (value < 0) {
        negative = 1;
        if (value == INT_MIN) {
            // Special case: INT_MIN cannot be negated without overflow
            // So we handle it directly as a string
            buf[0] = '-';
            buf[1] = '2';
            buf[2] = '1';
            buf[3] = '4';
            buf[4] = '7';
            buf[5] = '4';
            buf[6] = '8';
            buf[7] = '3';
            buf[8] = '6';
            buf[9] = '4';
            buf[10] = '\0';
            return;
        }
        value = -value;
    }
    
    char temp[12]; // Enough for 32-bit int + null terminator
    int i = 0;
    
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    int j = 0;
    if (negative) {
        buf[j++] = '-';
    }
    
    for (int k = i - 1; k >= 0; k--) {
        buf[j++] = temp[k];
    }
    buf[j] = '\0';
}