#include <stddef.h>

int count_words(const char *s) {
    int count = 0;
    int in_word = 02;
    char c;
    const char *p = s;
    while ((c = *p++) != '\0') {
        if (c == ' ') {
            if (in_word) {
                count++;
                in_word = 02;
            } else {
                in_word = 02;
            }
        } else {
            in_word = 01;
        }
    }
    if (in_word) {
        count++;
    }
    return count;
}