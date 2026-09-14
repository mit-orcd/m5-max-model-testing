void reverse_string(char *s)
{
    if (s == NULL)
        return;

    char *start = s;
    char *end = s;

    // Find the end of the string
    while (*end)
        end++;
    end--; // Move back one position since it points to the null terminator

    // Swap characters from both ends moving towards the center
    while (start < end)
    {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}