int count_words(const char *s) {
    int count = 0;
    int in_word = 0;
    if (s == NULL) return 0;
    for (; *s != '\0'; ++s) {
        if (*s != ' ') {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        } else {
            in_word = 0;
        }
    }
    return count;
}