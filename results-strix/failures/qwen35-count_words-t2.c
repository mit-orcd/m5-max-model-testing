#include <stdbool.h>

int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    bool in_word = false;

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