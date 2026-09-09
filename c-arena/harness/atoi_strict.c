
#include <stdio.h>
int atoi_strict(const char *s, int *out);
static int fails = 0;
static void ok(const char *s, int want) {
    int v = 0;
    if (atoi_strict(s, &v) != 0 || v != want) { printf("FAIL atoi_strict(\"%s\") v=%d want %d\n", s, v, want); fails++; }
}
static void bad(const char *s) {
    int v = 0;
    if (atoi_strict(s, &v) != -1) { printf("FAIL atoi_strict(\"%s\") should fail\n", s); fails++; }
}
int main(void) {
    ok("0",0); ok("42",42); ok("-42",-42); ok("007",7);
    bad(""); bad("abc"); bad("12x"); bad("-"); bad("1.5"); bad(" 12");
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

