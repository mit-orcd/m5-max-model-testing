int atoi_strict(const char *s, int *out)
{
    if (s == NULL || out == NULL)
        return -1;

    const char *p = s;

    if (*p == '+' || *p == '-')
        p++;

    if (*p == '\0')
        return -1;

    int sign = 1;
    if (*s == '-')
        sign = -1;

    long result = 0;
    while (*p != '\0') {
        if (*p < '0' || *p > '9')
            return -1;
        result = result * 10 + (*p - '0');
        p++;
    }

    *out = (int)(sign * result);
    return 0;
}