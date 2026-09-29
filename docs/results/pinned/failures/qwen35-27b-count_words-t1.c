#include <stdbool.h>

int count_words(const char *s) {
    int count = 0;
    bool in_word = false;

    if (s == NULL) {
        return 0;
    }

    while (*s != '\0') {
        if (*s != ' ') {
            if (!in_word) {
                count++;
                in_word = true;
            }
        } else {
            in_word = false;
        }
        s++;
    }

    return count;
}