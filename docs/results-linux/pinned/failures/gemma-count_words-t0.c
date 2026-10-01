#include <stdbool.h>
#include <ctype.h>

int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    bool in_word = false;

    while (*s != '\0') {
        if (isspace((unsigned char)*s)) {
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