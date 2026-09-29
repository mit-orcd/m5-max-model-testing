#include <ctype.h>

void trim(char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    char *p = s;
    while (*p) {
        p++;
    }
    while (p > s && isspace((unsigned char)*(p-1))) {
        p--;
    }
    *p = '\0';
}