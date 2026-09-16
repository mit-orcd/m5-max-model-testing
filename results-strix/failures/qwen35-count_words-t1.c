#include <stdbool.h>

int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    bool in_word = false;
    bool seen_char = false;

    while (*s) {
        if (*s == ' ') {
            if (in_word) {
                count++;
                in_word = false;
            }
            if (seen_char) {
                seen_char = true;
            }
        } else {
            if (!in_word) {
                count++;
                in_word = true;
            }
            seen_char = true;
        }
        s++;
    }

    if (in_word) {
        count++;
    }

    return count;
}