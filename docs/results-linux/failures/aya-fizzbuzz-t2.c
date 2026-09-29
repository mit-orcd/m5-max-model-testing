const char *fizzbuzz(int n) {
    static char s[12];
    if (n % 15 == 0) return "FizzBuzz";
    if (n % 3 == 0) return "Fizz";
    if (n % 5 == 0) return "Buzz";
    sprintf(s, "%d", n);
    return s;
}