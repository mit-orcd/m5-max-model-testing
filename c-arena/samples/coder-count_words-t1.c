int count_words(const char *s) {
    int count = 0;
    int in_word = 0;
    
    while (*s) {
        if (*s == ' ') {
            if (in_word) {
                inpublic: