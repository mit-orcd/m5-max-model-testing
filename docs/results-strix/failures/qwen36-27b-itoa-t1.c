#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char temp[12]; // Enough for INT_MIN: "-2147483648" (11 chars) + NUL
    int i = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value; // Note: this fails for INT_MIN, handled below
    }

    // Handle INT_MIN specially because -INT_MIN overflows
    if (value == 0 && is_negative) {
        // This branch handles INT_MIN
        // INT_MIN is -2147483648
        // We need to write "-2147483648"
        temp[0] = '2';
        temp[1] = '1';
        temp[2] = '4';
        temp[3] = '7';
        temp[4] = '4';
        temp[5] = '8';
        temp[6] = '3';
        temp[7] = '6';
        temp[8] = '4';
        temp[9] = '8';
        temp[10] = '\0';
        // Copy to buf with negative sign
        buf[0] = '-';
        for (int j = 0; j <= 10; j++) {
            buf[j + 1] = temp[j];
        }
        return;
    }

    // Extract digits in reverse order
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    // Copy to buf in correct order
    int j = 0;
    if (is_negative) {
        buf[j++] = '-';
    }
    for (int k = i - 1; k >= 0; k--) {
        buf[j++] = temp[k];
    }
    buf[j] = '\0';
}