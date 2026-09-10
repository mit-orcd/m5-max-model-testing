#include <ctype.h>

void trim(char *s) {
    char *end;

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return;
    }

    end = s + strlen(s) - 1;

    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    memmove(s, s, (end - s + 1));
}