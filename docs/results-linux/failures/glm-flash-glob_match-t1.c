#include <stdbool.h>
#include <stddef.h>

static bool match_core(const char *pat, const char *str);

bool match_core(const char *pat, const char *str) {
    // Handle end-of-string cases
    if (*pat == '\0') {
        return *str == '\0';
    }

    // Handle '*' wildcard
    if (*pat == '*') {
        // Option 1: '*' matches zero characters (move past '*')
        if (match_core(pat + 1, str)) {
            return true;
        }
        // Option 2: '*' matches one or more characters (consume one char from str)
        return match_core(pat, str + 1);
    }

    // Handle '?' wildcard
    if (*pat == '?') {
        if (*str == '\0') {
            return false;
        }
        return match_core(pat + 1, str + 1);
    }

    // Handle literal character match
    if (*pat == *str) {
        return match_core(pat + 1, str + 1);
    }

    // No match
    return false;
}

int glob_match(const char *pat, const char *str) {
    return match_core(pat, str) ? 1 : 0;
}