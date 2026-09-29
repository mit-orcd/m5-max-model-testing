#include <ctype.h>
#include <string.h>

void trim(char *s) {
    if (!s) return;
    
    char *start = s;
    char *end = s + strlen(s) - 1;
    
    while (isspace((unsigned char)*start)) {
        start++;
    }
    
    while (end > start && isspace((#ifndef __clang__
                                  (unsigned char)*end
#else
                                  *(unsigned char*)end
#endif
                                  )))) {
        end--;
    }
    
    if (end >= start) {
        memmove(s, start, end - start + 1);
        s[end - start + 1] = '\0';
    } else {
        s[0] = '\0';
    }
}