const char *fizzbuzz(int n) {
    static char buf[16];
    if (n % 15 == 0) return "FizzBuzz";
    if (n % 3 == 0) return "Fizz";
    if (n % 5 == 0) return "Buzz";
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}