#include <stdbool.h>
#include <stddef.h>

static bool match_star(const char *s, const char *p, const char *star);
static bool match(const char *s, const char *p);

static bool match_star(const char *s, const char *p, const char *star) {
    const char *s_temp = s;
    const char *p_temp = p + 1;
    
    if (*p_temp == '\0') {
        return true;
    }

    if (*p_temp == '?') {
        if (*s == '\0') {
            return false;
        }
        return match_star(s + 1, p_temp, star);
    }

    for (;;) {
        if (*s == '\0') {
            if (*p_temp == '*') {
                return match_star(s, p_temp, star);
            }
            return false;
        }

        if (*s == *p_temp) {
            s++;
            p_temp++;
        } else {
            if (*p_temp == '*') {
                return match_star(s, p_temp, star);
            }
            s = s_temp;
            p_temp = star + 1;
            s_temp++;
        }
    }
}

static bool match(const char *s, const char *p) {
    if (*p == '*') {
        return match_star(s, p, p);
    }
    
    if (*p == '?' || *p == *s) {
        if (*s == '\0') {
            return *p == '\0';
        }
        return match(s + 1, p + 1);
    }
    
    return false;
}

int glob_match(const char *pat, const char *str) {
    return match(str, pat);
}