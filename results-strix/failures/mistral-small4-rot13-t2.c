#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) return;
    for (int i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            s[i] = (c - base + 13) % 26 + base;
        }
    }
}