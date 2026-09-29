static char buf[12];
const char *fizzbuzz(int n) {
    if (n % 15 == 0) return "FizzBuzz";
    if (n % 3 == 0) return "Fizz";
    if (n % 5 == 0) return "Buzz";
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}