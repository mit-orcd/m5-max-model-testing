#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    memmove(s, start, end - start + 2);
}