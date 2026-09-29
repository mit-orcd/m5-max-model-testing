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
guarantee[5] = '4';
            buf[6] = '8';
            buf[7] = '\0';
            return;
        }
        value = -value;
    }
    
    int start = 0;
    if (negative) {
        buf[0] = '-';
        start = 1;
    }
    
    // Build the string in reverse order
    int i = start;
    while (value > 0) {
        buf[i] = '0' + (value % 10);
        value /= 10;
        i++;
    }
    buf[i] = '\0';
    
    // Reverse the digits (excluding the minus sign)
    int left = start;
    int right = i - 1;
    while (left < right) {
        char temp = buf[left];
        buf[left] = buf[right];
        buf[right] = temp;
        left++;
        right--;
    }
}