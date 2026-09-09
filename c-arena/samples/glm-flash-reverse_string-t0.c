#include <stddef.h>

void reverse_string(char *s) {
    if (s == NULL) {
        return;
    }

    char *start = s;
    char *end = s;
    char temp;

    while (*end != '\0') {
        end++;
    }
    end--; // Move back to the last character

    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp2;
        start++;
        end--;
    }
}