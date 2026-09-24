void reverse_string(char *s)
{
    if (s == NULL)
        return;

    size_t len = strlen(s);

    for (size_t i = 0, j = len - 1; i < j; i++, j--)
    {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}