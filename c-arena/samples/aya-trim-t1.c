#include <string.h>
#include <stdio.h>

void trim(char *s) {
    size_t len = strlen(s);
    size_t i, j;
    for (i = 0; i < len; i++) {
        if (!isspace(s[i])) {
            break;
        }
    }
    for (j = len - 1; j >= 0; j--) {
        if (!isspace(s[j])) {
            break;
        }
    }
    if (i > 0) {
        memmove(s, s + i, j - i + 1);
    }
    s[j + 1] = '\0';
}