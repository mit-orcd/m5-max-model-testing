void rot13(char *s) {
    if (s == NULL) return;
    for (; *s; ++s) {
        if (*s >= 'A' && *s <= 'Z') {
            *s = (*s - 'A' + 13) % 26 + 'A';
        } else if (*s >= 'a' && *s <= 'z') {
            *s = (*s - 'a' + 13) % 26 + 'a';
        }
    }
}