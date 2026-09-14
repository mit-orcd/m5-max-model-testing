void reverse_string(char *s)
{
    if (s == NULL)
        return;

    size_t len = 0;
    while (s[len] != '\0')
        len++;

    if (len == 0)
        return;

    size_t start = 0;
    size_t end = len - 1;

    while (start < end)
    {
        char tmp = s[start];
        s[start] = s[end];
        s[end] = tmp;
        start++;
        end--;
    }
}