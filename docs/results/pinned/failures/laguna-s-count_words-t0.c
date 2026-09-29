#include <stdbool>

int count_words(const char *s) {
    int count = 0;
    bool in_word = false;
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