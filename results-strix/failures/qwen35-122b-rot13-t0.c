#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) return;
    for (; *s != '\0'; ++s) {
        if (isalpha((unsigned char)*s)) {
            char base = isupper((unsigned char)*s) ? 'A' : 'a';
            *s = (char)(((*s - base + 13) % 26) + base);
        }
    }
}