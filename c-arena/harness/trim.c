
#include <stdio.h>
#include <string.h>
void trim(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); trim(buf);
    if (strcmp(buf, want)) { printf("FAIL trim(\"%s\")=\"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("  hello  ", "hello"); check("hello", "hello"); check("", "");
    check("   ", ""); check("\t hi \n", "hi"); check("a b", "a b");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

