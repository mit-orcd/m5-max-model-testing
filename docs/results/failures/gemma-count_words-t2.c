#include <ctype.h>

int count_words(const char *s) {
    int count = 0;
    int in_word = 0;

    if (s == NULL) {
        return 0;
    }

    for (int i = 0; s[i] != '\0'; i++) {
        if (isspace((unsigned char)s[i])) {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
    }

    return count;
}