#include <ctype.h>

void rot13(char *s) {
    if (s == NULL) {
        return;
    }

    while (*s != '\0') {
        if (isupper(*s)) {
            *s = 'A' + ((*s - 'A' + 13) % 26);
        } else if (islower(*s)) {
            *s = 'a' + ((*s - 'a' + 13) % 26);
        }
        s++;
    }
}