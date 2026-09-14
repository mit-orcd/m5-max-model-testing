#include <stdbool.h>

int count_words(const char *s) {
    if (s == NULL) return 0;

    bool in_word = false;
    int count = 0;

    while (*s) {
        if (*s == ' ') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            count++;
        }
        s++;
    }

    return count;
}