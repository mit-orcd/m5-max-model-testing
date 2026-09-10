#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start]))
        ++start;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1]))
        --end;
    if (start == 0 && end == len)
        return;
    memmove(s, s + start, end - start + 1);
}