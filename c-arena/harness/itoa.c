
#include <stdio.h>
#include <string.h>
#include <limits.h>
void itoa(int value, char *buf);
static int fails = 0;
static void check(int v) {
    char buf[32], want[32];
    itoa(v, buf); snprintf(want, sizeof want, "%d", v);
    if (strcmp(buf, want)) { printf("FAIL itoa(%d)=\"%s\" want \"%s\"\n", v, buf, want); fails++; }
}
int main(void) {
    check(0); check(7); check(-7); check(12345); check(-98765); check(INT_MAX); check(INT_MIN);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

