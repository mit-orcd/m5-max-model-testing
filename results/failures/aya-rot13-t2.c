#include <string.h>

void rot13(char *s) {
    for (char *p = s; *p; p++) {
        if (isalpha(*p)) {
            if (islower(*p)) {
                *p = (char)(tolower(*p) + 13) % 26 + 'a';
            } else {
                *p = (char)(toupper(*p) + 13) % 26 + 'A';
            }
        }
    }
}