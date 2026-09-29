#include <ctype.h>

void rot13(char *s) {
    for (; *s; s++) {
        if (isalpha((unsigned char)*s)) {
            *s = (char)((*s - 'a' + 13) % 26 + 'a');
        }
    }
}