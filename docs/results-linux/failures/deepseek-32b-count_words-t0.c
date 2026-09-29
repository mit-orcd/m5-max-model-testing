int count_words(const char *s) {
    int count = 0;
    bool in_word = false;
    for (; *s != '\0'; s++) {
        if (*s == ' ') {
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