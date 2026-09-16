void reverse_string(char *s)
{
    char *end = s;
    char temp;

    if (s == NULL) {
        return;
    }

    while (*end != '\0') {
        ++end;
    }

    --end;
    while (s < end) {
        temp = *s;
        *s++ = *end;
        *end-- = temp;
    }
}