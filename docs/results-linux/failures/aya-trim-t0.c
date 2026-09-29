#include <string.h>

void trim(char *s) {
    size_t i, j;
    for (i = 0; s[i] && strchr(" \t\n", s[i]); i++)
        ;
    for (j = strlen(s) - 1; j >= i && strchr(" \t\n", s[j]); j--)
        ;
    s[j + 1] = '\0';
}