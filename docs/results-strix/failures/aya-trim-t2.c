#include <stdio.h>
#include <string.h>

void trim(char *s) {
    size_t len = strlen(s);
    size_t i;
    for (i = 0; i < len; i++) {
        if (!isspace(s[i])) {
            memmove(s, s + i, len - i + 1);
            s[len] = '\0';
            break;
        }
    }
    for (i = len - 1; i >= 0; i--) {
        if (!isspace(s[i])) {
            s[i + 1] = '\0';
            break;
        }
    }
}