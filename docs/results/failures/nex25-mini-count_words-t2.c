int count_words(const char *s)
{
    if (s == NULL) {
        return 0;
    }

    int count = 0;
    int in_word = 0;

    for (; *s; ++s) {
        if (*s == ' ') {
            if (in_word) {
                ++count;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }

    if (in_word) {
        ++count;
    }

    return count;
}