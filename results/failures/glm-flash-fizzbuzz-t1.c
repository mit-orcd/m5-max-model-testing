#include <stddef.h>

const char *fizzbuzz(int n) {
    static char buffer[32];
    if (n % 15 == 0) {
        __builtin_memcpy(buffer, "FizzBuzz", 8);
    } else if (n % 5 == 0) {
        __builtin_memcpy(buffer, "Buzz", 4);
    } else if (n % 3 == 0) {
        __builtin_memcpy(buffer, "Fizz", 4);
    } else {
        int d = 10;
        char *p = buffer;
        if (n < 0) { *p++ = '-'; n = -n; }
        if (n >= d) {
            while ((d *= 10) <= n) {}
            d /= 10;
        } else {
            d = 10;
        }
        do { *p++ = '0' + n / d; } while ((n %= d) && (d /= 10));
        *p = '\0';
        __builtin_memcpy(buffer, p, (size_t)(p - buffer));
        return buffer;
    }
    return buffer;
}