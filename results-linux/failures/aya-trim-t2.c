#include <string.h>

void trim(char *s) {
    size_t len = strlen(s);
    while (isspace((unsigned char)s[len - 1])) s[--len] = 0;
    while (isspace((unsigned char)*s)) s += strspn(s, " \t\n");
}