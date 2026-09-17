#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) {
        return;
    }

    while (*s) {
        if (isalpha((unsigned char)*s)) {
            char base = isupper((unsigned char)*s) ? 'A' : 'a';
            *s = base + (*s - base + 13) % 26;
        }
        s++;
    }
}