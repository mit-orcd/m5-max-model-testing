int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    int in_word = 0;

    while (*s != '\0') {
        if (*s == ' ') {
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