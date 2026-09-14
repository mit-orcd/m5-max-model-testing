#include <stdint.h>
#include <stdbool.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    bool negative = value < 0;
    if (negative && value == INT_MIN) {
        // Special case for INT_MIN to avoid overflow when negating
        int32_t temp = value;
        int i = 0;
        while (temp != 0) {
            buf[i++] = '0' - (temp % 10);
            temp /= 10;
        }
        buf[i] = '\0';
        // Reverse the string
        for (int j = 0; j < i/2; j++) {
            char c = buf[j];
            buf[j] = buf[i-1-j];
            buf[i-1-j] = c;
        }
        buf[i] = '\0';
        return;
    }
    
    if (negative) {
        value = -value;
    }
    
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (negative) {
        buf[i++] = '-';
    }
    
    // Reverse the string
    for (int j = 0; j < i/2; j++) {
        char c = buf[j];
        buf[j] = buf[i-1-j];
        buf[i-1-j] = c;
    }
    
    buf[i] = '\0';
}