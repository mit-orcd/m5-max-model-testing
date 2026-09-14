#include <stddef.h>

int glob_match(const char *pat, const char *str)
{
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*s != '\0') {
        if (*p == '\0') {
            return 0;
        }
        if (*p == '*') {
            star = p;
            ss = s;
            p++;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else if (star != NULL) {
            /* Backtrack: match '*' with one more character */
            p = star + 1;
            ss++;
            s = ss;
        } else {
            return 0;
        }
    }

    /* Skip trailing '*' in pattern */
    while (*p == '*') {
        p++;
    }

    return *p == '\0' ? 1 : 0;
}