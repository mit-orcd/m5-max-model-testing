#include <ctype.h>
#include <string.h>

void trim(char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }

    size_t len = (size_t)(end - s + 1);
    memmove(s, s + (strlen(s) - len), len + 1);
}