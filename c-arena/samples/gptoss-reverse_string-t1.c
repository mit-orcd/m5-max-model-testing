#include <string.h>

void reverse_string(char *s) {
    size_t len = strlen(s);
    size_t i = 0, j = len - 1;
    while (i < j) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        ++i;
        --j;
    }
}