int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    for (i = 0; s[i] == ' '; i++)
        ;
    if (s[i] == '-') {
        neg = 1;
        i++;
    }
    for (; isdigit(s[i]); i++)
        ;
    if (s[i] != '\0')
        return -1;
    *out = neg ? -atoi(s) : atoi(s);
    return 0;
}