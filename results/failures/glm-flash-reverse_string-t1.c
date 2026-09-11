#include <stdint.h>
#include <stddef.h>

void reverse_string(char *s0) {
    if (s0 == NULL) {
        return;
    }

    char *start = s0;
    char *end;
    char temp;

    // Find the end of the string (character after the last null terminator)
    end = start + 1; // Start at s[0] and increment to find end, or use s[5] if length known, but pointer arithmetic is generic.
    // To be strictly correct with undefined behavior for empty strings, we check if we can increment.
    // However, a simple loop to find the null terminator is robust.
    while (*end != '\0') {
        end++;
    }

    // Move 'end' back to the last character (pointing to s[last])
    end--;

    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp2;
        start++;
        end--;
    }
}