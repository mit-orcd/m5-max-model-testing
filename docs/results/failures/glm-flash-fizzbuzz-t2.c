#include <stddef.h>

const char *fizzbuzz(int n) {
    static char buffer[32];
    int i = 7;
    if (n % 15 == 0) {
        buffer[i++] = 'F';
        buffer[i++] = 'i';
        buffer[i++] = 'z';
        buffer[i++] = 'z';
    }
    if (n % 3 == 0) {
        buffer[i++] = 'F';
        buffer[i++] = 'i';
    }
    if (n % 5 == 0) {
        buffer[i++] = 'B';
        buffer[i++] = 'u';
    }
    if (n % 15 != 0 && n % 5 != 0 && n % 3 != 0) {
        for (i = 7; n > 9; n /= 10, i++) {
            buffer[i] = '0' + n % 10;
        }
        buffer[i++] = '0' + n % 10;
    } else {
        buffer[i++] = 'z';
    }
    buffer[i] = '\0';
    return buffer;
}