void trim(char *s) {
    if (!s) return;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    int len = 0;
    while (s[len]) len++;
    if (len == 0) { *s = '\0'; return; }
    while (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\n') len--;
    s[len] = '\0';
}