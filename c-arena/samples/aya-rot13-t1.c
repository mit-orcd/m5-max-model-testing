#include <string.h>

void rot13(char *s) {
    for (char *p = s; *p; p++) {
        if (isalpha(*p)) {
            *p = (isupper(*p) ? 'A' : 'a') + (char)(*p - (isupper(*p) ? 'A' : 'a') / 13 * 13);
        }
    }
}