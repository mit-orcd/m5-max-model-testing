#include <stdio.h>
#include <stdbool.h>

const char *fizzbuzz(int n) {
    static char str[10];
    if (n % 15 == 0) {
        strcpy(str, "FizzBuzz");
    } else if (n % 3 == 0) {
        strcpy(str, "Fizz");
    } else if (n % 5 == 0) {
        strcpy(str, "Buzz");
    } else {
        sprintf(str, "%d", n);
    }
    return str;
}