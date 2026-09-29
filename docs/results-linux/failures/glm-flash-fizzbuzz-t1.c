#include <string.h>

const char *fizzbuzz(int n) {
    static char buffer[32];
    int rem15 = n % 15;
    if (rem15 == 0) {
        return "FizzBuzz";
    }
    if (n % 3 == 0) {
        return "Fizz";
    }
    if (n % 5 == 0) {
        return "Buzz";
    }
    int len = snprintf(buffer, sizeof(buffer), "%d", n);
    return (len > 0 && len < sizeof(buffer)) ? buffer : "0";
}