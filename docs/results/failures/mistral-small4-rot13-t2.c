#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) return;
    for (char *p = s; *p != '\0'; p++) {
        char c = *p;
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c -= base;
            c += (c + 13) % 26;
            *p = c + base;
        }
    }
}