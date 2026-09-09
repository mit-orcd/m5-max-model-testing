
#include <stdio.h>
int parse_csv_ints(const char *s, int *out, int max);
static int fails = 0;
static void check(const char *s, int max, const int *want, int wantn) {
    int out[16]; int got = parse_csv_ints(s, out, max);
    if (got != wantn) { printf("FAIL parse(\"%s\") count %d want %d\n", s, got, wantn); fails++; return; }
    for (int i = 0; i < wantn; i++)
        if (out[i] != want[i]) { printf("FAIL parse(\"%s\")[%d]=%d want %d\n", s, i, out[i], want[i]); fails++; return; }
}
int main(void) {
    int w1[] = {10,20,30}; int w2[] = {-5}; int w3[] = {1,2};
    check("10,20,30", 16, w1, 3); check("-5", 16, w2, 1);
    check("1,2,3,4,5", 2, w3, 2); check("", 16, 0, 0);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

