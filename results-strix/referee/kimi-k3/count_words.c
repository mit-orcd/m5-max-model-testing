int count_words(const char *s) {
    int count = 0, in_word = 0;
    for (; *s; s++) {
        if (*s == ' ') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            count++;
        }
    }
    return count;
}
