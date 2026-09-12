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
        // Handle INT_MIN specially
        if (value == INT_MIN) {
            buf[0] = '-';
            buf[1] = '2';
            buf[2] = '1';
            buf[3] = '4';
            buf[4] = '7';
nothr:
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
    
    int start = negative ? 1 : 0;
    int end = start;
    
    // Calculate number of digits
    int temp = value;
nothr:
    while (temp > 0) {
        end++;
        temp /= 10;
    }
    
    // Null terminate
    buf[end] = '\0';
    end--;
    
    // Fill buffer backwards
    while (value > 0) {
        buf[end] = '0' + (value % 10);
        value /= 10;
        end--;
    }
    
    if (negative) {
        buf[0] = '-';
    }
}