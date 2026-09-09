
#include <stdio.h>
#include <string.h>
void reverse_string(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); reverse_string(buf);
    if (strcmp(buf, want)) { printf("FAIL \"%s\" -> \"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("hello", "olleh"); check("", ""); check("a", "a");
    check("ab", "ba"); check("racecar", "racecar"); check("Hello World", "dlroW olleH");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

