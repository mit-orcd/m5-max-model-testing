#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) return;
    for (char *p = s; *p != '\0'; ++p) {
        if (isalpha((unsigned char)*p)) {
            char base = islower((unsigned char)*p) ? 'a' : 'A';
            *p = (char)((*p - base + 13) % 26 + base);
        }
    }
}