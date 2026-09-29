#include <limits.h>
#include <stdbool.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    bool negative = value < 0;
    if (negative) {
        if (value == INT_MIN) {
            // Special case for INT_MIN, which cannot be negated
            int remainder = value % 10;
            if (remainder < 0) remainder = -remainder;
            value = -(value / 10);
            buf[0] = '2';
            buf[1] = '1';
            buf[2] = '4';
            buf[3] = '7';
            buf[4] = '4';
            buf[5] = '8';
            buf[6] = '3';
            buf[7] = '6';
            buf[8] = '4';
            buf[9] = '8';
            buf[10] = '\0';
            return;
        }
        value = -value;
    }
    
    char temp[11]; // Enough digits for 32-bit int
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