const char *fizzbuzz(int n) {
    static char buf[12];
    if (n % 15 == 0) return "FizzBuzz";
    if (n % 3 == 0) return "Fizz";
    if (n % 5 == 0) return "Buzz";
    return itoa(n, buf, 10);
}