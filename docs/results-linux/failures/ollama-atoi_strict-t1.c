```c
int atoi_strict(const char *s, int *out)
{
    if (s == NULL)
        return -1;

    int i = 0;
    int neg = 0;

    if (s[0] == '-') {
        neg = 1;
        i = 1;
    }

    if (s[i] < '0' || s[i] > '9')
        return -1;