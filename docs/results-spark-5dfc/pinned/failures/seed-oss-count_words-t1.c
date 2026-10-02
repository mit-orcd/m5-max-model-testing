int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }
    int word_count = 0;
    int in_word = 0;
    while (*s != '\0') {
        if (*s == ' ') {
            if (in_word) {
                word_count++;
                in_word = 0;
            }
        } else {
            if (!in_word) {
                in_word = 1;
            }
        }
        s++;
    }
    if (in_word) {
        word_count++;
    }
    return word_count;
}