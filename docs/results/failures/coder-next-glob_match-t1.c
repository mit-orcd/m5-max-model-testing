#include <ctype.h>

int glob_match(const char *pat, const char *str) {
    if (!*pat) {
        return !*str;
    }
    
    if (*pat == '*') {
        // Skip consecutive '*' characters
        while (*pat == '*') {
            pat++;
        }
        
        // Try matching empty string
        if (!*pat) {
            return !*str;
        }
        
        // Try matching with '*' consuming 0 or more characters
        do {
            if (glob_match(pat, str)) {
                return 1;
            }
            if (!*str) {
                break;
            }
            str++;
        } while (1);
        
        return 0;
    }
    
    if (!*str) {
        return 0;
    }
    
    if (*pat == '?') {
        return glob_match(pat + 1, str + 1);
    }
    
    if (*pat != *str) {
        return 0;
    }
    
    return glob_match(pat + 1, str + 1);
}