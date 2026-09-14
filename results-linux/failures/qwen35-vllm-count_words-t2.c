#include <stdbool.h>

int count_words(const char *s) {
    if (s == NULL) return 0;

    int count = 0;
    bool in_word = false;

    while (*s) {
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