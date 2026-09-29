#include <string.h>

int glob_match(const char *pat, const char *str) {
    int pat_len = strlen(pat), str_len = strlen(str);
    int i, j;
    for (i = 0, j = 0; i < pat_len && j < str_len; i++) {
        if (pat[i] == '*') {
            while (j < str_len && pat[i] == '*') i++;
            if (i == pat_len) break;
            j = 0;
        } else if (pat[i] == '?') {
            if (j < str_len) j++;
        } else {
            if (str[j] != pat[i]) return 0;
            j++;
        }
    }
    return i == pat_len && j == str_len;
}