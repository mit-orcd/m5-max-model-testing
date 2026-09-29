#include <string.h>

void trim(char *s) {
    size_t i, j;
    for (i = 0; s[i] && strchr(" \t\n", s[i]); i++)
        ;
    for (j = strlen(s) - 1; j >= 0 && strchr(" \t\n", s[j]); j--)
        ;
    s[j + 1] = '\0';
    memmove(s, s + i, j - i + 1);
}