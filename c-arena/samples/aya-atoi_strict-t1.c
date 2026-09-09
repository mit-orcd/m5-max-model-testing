int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    for (i = 0; s[i] == ' '; i++)
        ;
    if (i == strlen(s))
        return -1;
    if (s[i] == '-') {
        neg = 1;
        i++;
    }
    for (; s[i] >= '0' && s[i] <= '9'; i++)
        ;
    if (i == strlen(s) || s[i] != '\0')
        return -1;
    *out = neg ? -atoi(s) : atoi(s);
    return 0;
}