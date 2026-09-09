
#include <stdio.h>
int count_words(const char *s);
static int fails = 0;
static void check(const char *s, int want) {
    int got = count_words(s);
    if (got != want) { printf("FAIL count_words(\"%s\")=%d want %d\n", s, got, want); fails++; }
}
int main(void) {
    check("hello",1); check("hello world",2); check("the quick  brown fox",4);
    check("",0); check("   ",0); check("  pad  ",1); check("a b c d e",5);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

