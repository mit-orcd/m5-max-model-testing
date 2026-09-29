#include <stdint.h>
#include <stdbool.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    bool negative = value < 0;
    if (negative) {
        if (value == INT32_MIN) {
            // Special case: INT32_MIN cannot be negated without overflow
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