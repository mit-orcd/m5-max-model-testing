int count_words(const char *s) {
    int count = 0;
    bool in_word = false;
    while (*s) {
        if (*s != ' ' && !in_word) {
            count++;
            in_word = true;
        } else if (*s == ' ') {
            in_word = false;
        }
        s++;
    }
    return count;
}