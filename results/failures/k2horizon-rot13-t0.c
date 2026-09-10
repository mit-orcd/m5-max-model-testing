#include <ctype.h>

void rot13(char *s) {
    for (; *s; s++) {
        if (isalpha((unsigned char)*s)) {
            *s = (char)((( *s - 'A' + 13) % 26) + 'A');
        }
    }
}