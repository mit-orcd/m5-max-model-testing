#include <ctype.h>

int count_words(const char *s) {
    int count = 0;
    int in_word = 0;

    if (s == NULL) {
        return 0;
    }

    for (const char *p = s; *p != '\0'; p++) {
        if (isspace((unsigned char)*p)) {
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