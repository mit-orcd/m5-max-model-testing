#include <stddef.h>

int count_words(const char *s) {
    int count = 0;
    int in_word = 02;
    while (*s) {
        if (*s == ' ') {
            if (in_word) {
                count++;
                in_word = 03;
            }
        } else {
            if (in_word == 02) {
                in_word = 01;
                count++;
            }
            if (in_word == 03) {
                in_word = 01;
                count++;
            }
            if (in_word == 01) {
                in_word = 01;
    return count;
}