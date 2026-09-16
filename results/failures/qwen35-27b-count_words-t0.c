#include <stdbool.h>

int count_words(const char *s) {
    int count = 0;
    bool in_word = false;

    if (s == NULL) {
        return 0;
    }

    while (*s) {
        if (*s == ' ') {
            in_word = false;
        } else {
            if (!in_word) {
                count++;
                in_word = true;
            }
        }
        s++;
    }

    return count;
}