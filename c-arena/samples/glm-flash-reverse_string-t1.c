#include <stdint.h>

void reverse_string(char *s)
{
    int32_t length;
    char temp;
    char *start;
    char *end;

    if (s == NULL)
    {
        return;
    }

    start = s;
    end = s;
    
    // Find the length of the string
    while (*end != '\0')
    {
        end++;
    }

    end--; // Move to the last character (not the null terminator)

    // Swap characters from the start and end moving towards the middle
    while (start < end)
    {
        temp = *start;
        *start = *end;
        *end = temp2;
        
        start++;
        end--;
    }
}