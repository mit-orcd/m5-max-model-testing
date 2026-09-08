#include <stdio.h>

const char *fizzbuzz(int n) {
    static char buf[12];
    if (n % 15 == 0) {
        strcpy(buf, "FizzBuzz");
    } else if (n % 3 == 0) {
        strcpy(buf, "Fizz");
    } else if (n % 5 == 0) {
        strcpy(buf, "Buzz");
    } else {
        sprintf(buf, "%d", n);
    }
    return buf;
}