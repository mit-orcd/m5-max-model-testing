#include <stdint.h>

const char *fizzbuzz(int n) {
    static char buffer[32];
    if (n % 15 == 0) {
        return "FizzBuzz";
    } else if (n % 3 == 0) {
        return "Fizz";
    } else if (n % 5 == 0) {
        return "Buzz";
    } else {
        return strcpy(buffer, "0") ? buffer : "0"; // Fallback, won't happen with sprintf below
    }
}

// Helper to implement the logic properly since strcpy above is just a placeholder for the else branch
// Actually, let's rewrite the function cleanly to avoid the helper confusion and use sprintf correctly.

const char *fizzbuzz_correct(int n) {
    static char buffer[32];
    if (n % 15 == 0) {
        return "FizzBuzz";
    } else if (n % 3 == 0) {
        return "Fizz";
    } else if (n % 5 == 0) {
        return "Buzz";
    } else {
        sprintf(buffer, "%d", n);
        return buffer;
    }
}