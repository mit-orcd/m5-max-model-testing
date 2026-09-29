#include <string.h>

void reverse_string(char *s) {
    size_t n = strlen(s);
    for (size_t i = 0, j = n ? n - 1 : 0; i < j; i++, j--) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}
