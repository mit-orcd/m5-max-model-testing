int count_words(const char *s) {
    if (s == NULL) return 0;
    int word_count = 0;
    int in_word = 0;
    while (*s) {
        if (*s != ' ') {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
        s++;
    }
    return word_count;
}