#include <string.h>

void rot13(char *s) {
    for (char *c = s; *c; c++) {
        if (isalpha(*c)) {
            *c = (islower(*c) ? 'a' : 'A') + (*c - (islower(*c) ? 'a' : 'A') + 13) % 26;
        }
    }
}