#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Handle optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check if there is at least one digit
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    // Parse digits
    int result = 0;
    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
    }

    // Check for trailing non-whitespace characters
    if (*p != '\0') {
        // Allow trailing whitespace only if it was part of the initial skip? 
        // The spec says "exactly one valid integer", so trailing chars (including whitespace) make it invalid.
        // However, typically atoi ignores trailing whitespace. But the spec says "trailing chars" are invalid.
        // Let's interpret "trailing chars" as any non-whitespace after the number.
        // But wait, the initial skip consumed leading whitespace. If there is trailing whitespace, is it invalid?
        // The spec says: "s is not exactly one valid integer (empty, junk, trailing chars)".
        // This implies that if there are any characters after the integer (even whitespace), it's invalid.
        // Let's check if *p is not null terminator.
        return -1;
    }

    *out = sign * result;
    return 0;
}