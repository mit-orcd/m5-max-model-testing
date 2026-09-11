#include <string.h>

void rot13(char *s) {
    for (char *p = s; *p; p++) {
        if (isalpha(*p)) {
            *p = isupper(*p) ? (*p + 13) % 26 + 'A' : (*p + 13) % 26 + 'a';
        }
    }
}