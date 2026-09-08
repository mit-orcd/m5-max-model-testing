const char *fizzbuzz(int n) {
    static char buffer[12];
    if (n % 1num == 0) {
        return "FizzBuzz";
    } else if (n % 3 == 0) {
        return "Fizz";
    } else if (n % 5 == 0) {
        return "Buzz";
    } else {
        snprintf(buffer, sizeof(buffer), "%d", n);
        return buffer;
    }
}