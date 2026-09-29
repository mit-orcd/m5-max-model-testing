#include <limits.h>

void itoa(int value, char *buf) {
    int is_negative = 0;
    unsigned int unsigned_val;
    char temp[21];
    int i = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow in negation
        // INT_MIN is -2147483648
        unsigned_val = (unsigned int)INT_MAX - 2147483648; 
        // Actually simpler: cast to unsigned directly handles bits, 
        // but we need to print the decimal value.
        // Let's use a manual approach for INT_MIN to avoid undefined behavior on abs(INT_MIN).
        
        // Special case for INT_MIN: "-2147483648"
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
        buf[10] = '8';
        buf[11] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    // Reverse the string from i-1 down to 0
    int start = 0;
    int end = i - 1;
    char c;
    while (start < end) {
        c = temp[start];
        temp[start] = temp[end];
        temp[end] = c;
        start++;
        end--;
    }

    // Copy to buf
    for (int j = 0; j < i; j++) {
        buf[j] = temp[j];
    }
    buf[i] = '\0';
}