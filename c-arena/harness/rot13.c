
#include <stdio.h>
#include <string.h>
void rot13(char *s);
static int fails = 0;
static void check(const char *in, const char *want) {
    char buf[256]; strcpy(buf, in); rot13(buf);
    if (strcmp(buf, want)) { printf("FAIL rot13(\"%s\")=\"%s\" want \"%s\"\n", in, buf, want); fails++; }
}
int main(void) {
    check("hello", "uryyb"); check("uryyb", "hello"); check("ABC xyz", "NOP klm");
    check("", ""); check("123!", "123!"); check("Zebra", "Mroen");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

