int count_words(const char *s) {
    int count = 0;
    int in_word = 0;

    if (s == NULL) {
        return 0;
    }

    while (*s) {
        if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v') {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
        s++;
    }

    return count;
}