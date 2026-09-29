#include <ctype.h>

void rot13(char *s)
{
    if (s == NULL)
        return;
    for (char *p = s; *p; ++p) {
        if (isalpha((unsigned char)*p)) {
            if ('a' <= *p && *p <= 'z')
                *p = 'a' + ((*p - 'a' + 13) % 26);
            else if ('A' <= *p && *p <= 'Z')
                *p = 'A' + ((*p - 'A' + 13) % 26);
        }
    }
}