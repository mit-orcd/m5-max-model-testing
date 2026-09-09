
#include <stdio.h>
#include <string.h>
const char *fizzbuzz(int n);
static int fails = 0;
static void check(int n, const char *want) {
    const char *got = fizzbuzz(n);
    if (strcmp(got, want)) { printf("FAIL fizzbuzz(%d)=\"%s\" want \"%s\"\n", n, got, want); fails++; }
}
int main(void) {
    check(1,"1"); check(3,"Fizz"); check(5,"Buzz"); check(15,"FizzBuzz");
    check(7,"7"); check(30,"FizzBuzz"); check(9,"Fizz"); check(100,"Buzz"); check(-3,"Fizz");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

