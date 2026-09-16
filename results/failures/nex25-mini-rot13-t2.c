void rot13(char *s)
{
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = (char)(*s - 'a' + 13) + 'a';
        else if (*s >= 'A' && *s <= 'Z')
            *s = (char)(*s - 'A' + 13) + 'A';
        ++s;
    }
}