#include <string.h>

void trim(char *s) {
    size_t len = strlen(s);
    size_t i;
    for (i = 0; i < len; i++) {
        if (!isspace(s[i])) {
            break;
        }
    }
    s += i;
    len -= i;
    for (i = len - 1; i >= 0; i--) {
        if (!isspace(s[i])) {
            break;
        }
    }
    s[i + 1] = '\0';
}