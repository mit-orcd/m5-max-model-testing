#include <stdio.h>
#include <string.h>

void trim(char *s) {
    size_t i, j;
    for (i = 0; s[i] && strchr(s[i], " \t\n"); i++)
        ;
    for (j = strlen(s) - 1; strchr(s[j], " \t\n"); j--)
        ;
    s[j + 1] = 0;
    memmove(s, s + i, j - i + 1);
}