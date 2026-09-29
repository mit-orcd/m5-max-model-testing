const char *fizzbuzz(int n) {
    static char buffer[16];
    int is_fizz = (n % 3 == 0);
    int is_buzz = (n % 5 == 0);

    if (is_fizz && is_buzz) {
        return "FizzBuzz";
    } else if (is_fizz) {
        return "Fizz";
    } else if (is_buzz) {
        return "Buzz";
    } else {
        return itoa(n, buffer);
    }
}