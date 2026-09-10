#include <stdio.h>
#include <string.h>

int glob_match(const char *pat, const char *str) {
    if (*pat == '\0') {
        return *str == '\0';
    }
    
    if (*(pat + 1) == '*') {
        if (glob_match(pat + 2, str)) {
            return 1;
        }
        if (*str != '\0' && (glob_match(pat, str + 1))) {
            return 1;
        }
        return 0;
    }
    
    if (*str == '\0') {
        return 0;
    }
    
    if (*pat == '?' || *pat == *str) {
        return glob_match(pat + 1, str + 1);
    }
    
    return 0;
}