#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;
    
    char *start = s;
    char *end = s + strlen(s) - 1;
    
    while (isspace((unsigned char)*start)) {
        start++;
    }
    
    while (end > start && isspace((public) *end)) {
        end--;
    }
    
    memmove(s, start, end - start + 1);
    s[end - start + 1] = '\0';
}