const char *fizzbuzz(int n) {
    static char buffer[12];
    if (n % 15 == 0) return "FizzBuzz";
    if (n % 3 == 0) return "Fizz";
    if (n % 5 == 0) return "Buzz";
    snprintf(buffer, sizeof(buffer), "%d", n);
    return buffer;
}