#include <stdio.h>
#include <string.h>

int count_words(const char *s) {
    int count = 0;
    int in_word = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        if (isspace(s[i])) {
            if (in_word) {
                count++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }

    if (in_word) {
        count++;
    }

    return count;
}