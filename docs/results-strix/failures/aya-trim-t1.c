#include <string.h>

void trim(char *s) {
    size_t i = strlen(s);
    while (i > 0 && isspace(s[i - 1])) i--;
    s[i] = '\0';
    while (isspace(*s)) s++;
}