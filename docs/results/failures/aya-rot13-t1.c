void rot13(char *s) {
    for (char *p = s; *p; p++) {
        if (isalpha(*p)) {
            *p = isupper(*p) ? 'A' + (*p - 'A' + 13) % 26 : 'a' + (*p - 'a' + 13) % 26;
        }
    }
}