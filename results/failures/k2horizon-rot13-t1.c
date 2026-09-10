#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) return;
    for (; *s != '\0'; ++s) {
        if (isalpha((unsigned char)*s)) {
            *s = (char)((*s - 'a' + 13) % 26 + 'a');
        }
    }
}