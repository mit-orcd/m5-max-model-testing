#include <ctype.h>

int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    int in_word = 0;
    int i = 0;

    while (s[i] != '\0') {
        if (isspace((unsigned char)s[i])) {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
        i++;
    }

    return count;
}