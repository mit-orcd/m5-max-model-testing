#include <string.h>

int glob_match(const char *pat, const char *str) {
    int p_len = strlen(pat), s_len = strlen(str);
    int i, j;
    for (i = 0, j = 0; i < p_len && j < s_len; i++) {
        if (pat[i] == '*') {
            while (i < p_len && pat[i] == '*') i++;
            while (j < s_len && pat[i] == '*') j++;
        } else if (pat[i] == '?') {
            j++;
        } else {
            if (pat[i] != str[j]) return 0;
            j++;
        }
    }
    return i == p_len && j == s_len;
}