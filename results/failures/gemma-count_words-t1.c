#include <stdbool.h>
#include <ctype.h>

int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    bool in_word = false;

    for (int i = 0; s[i] != '\0'; i++) {
        if (isspace((unsigned char)s[i])) {
            in_word = false;
        } else {
            if (!in_word) {
                count++;
                in_word = true;
            }
        }
    }

    return count;
}