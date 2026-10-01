#include <limits.h>
#include <string.h>

void itoa(int value, char *buf) {
    int i = 0;
    int negative = 0;
    
    if (value < 0) {
        negative = 1;
        // Handle INT_MIN safely by using long long for negation
        long long val = (long long)value;
        val = -val;
        value = (int)val;
    }
    
    // Handle zero case
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    // Convert digits in reverse order
    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    
    // Add negative sign if needed
    if (negative) {
        buf[i++] = '-';
    }
    
    // Null-terminate and reverse the string
    buf[i] = '\0';
    
    // Reverse the buffer in place
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        start++;
        end--;
    }
}