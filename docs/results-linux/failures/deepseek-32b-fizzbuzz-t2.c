const char *fizzbuzz(int n) {
    static char buffer[20];
    if (n % 15 == 0) {
        strcpy(buffer, "FizzBuzz");
    } else if (n % 3 == 0) {
        strcpy(buffer, "Fizz");
    } else if (n % 5 == 0) {
        strcpy(buffer, "Buzz");
    } else {
        snprintf(buffer, sizeof(buffer), "%d", n);
    }
    return buffer;
}