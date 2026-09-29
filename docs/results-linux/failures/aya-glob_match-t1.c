#include <string.h>

int glob_match(const char *pat, const char *str) {
    int i, j, n = strlen(pat);
    for (i = 0; i < n; i++) {
        if (pat[i] == '*') {
            if (i == n - 1) return 1;
            j = i + 1;
            while (j < n && pat[j] != '*') j++;
            if (j == n) return 1;
            i = j;
        } else if (pat[i] == '?') {
            if (i == n - 1 || str[i]) return 1;
        } else {
            if (i == n - 1 || pat[i] != str[i]) return 0;
        }
    }
    return !strcmp(pat, str);
}