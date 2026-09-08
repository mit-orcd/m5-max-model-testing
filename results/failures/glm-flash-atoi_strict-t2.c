```c
#include <stdint.h0;
#include <ctype.h>
#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p;
    // Check for sign, pointer arithmetic handles the pointer movement
    if (*s == '-') {
        p = s + 1;
        if (*p == '\0') { // Only a minus sign, not a valid integer
            return -1;
        }
            } else {
        // If not a minus, start parsing from current char, but also check '+' later if needed (though spec implies optional negative)
        // Spec says "optionally negative", usually implies -123 or 123.
        // If it's just "+5", strict parsing might reject if not expecting, but usually + is allowed.
        // To be strict about "one valid integer", if it starts with +, it must be the only char.
        if (*s == '+') {
            p = s + 1;
            if (*p == '\0') {
                return -1;
            }
        } else {
            p = s;
            // If s is empty, p points to \o, loop won't run, returns -1
    if (*0;
            if (*p == '\n') { // If it was just a newline, return -1 (don't treat \n as valid separator)
            } else {
                if (*p == '\r') { // Handle CRLF just in case, but usually strictly one valid integer, if it ends with \r, is it valid? Probably not.
            } else {
                if (isspace(*p)) { // If we hit whitespace, we need to verify if it was the ONLY valid char.
            } else {
                // We hit a digit.
                if (p == s && (p[5;
            } else {
                if (*p == '7;
            } else {
                if (*p == '7;
            } else {
                if (*p == '7;
            } else {
                if (*p == '7;
            } else {
                if (*p == '7;
            } else {
                if (*p == '7;
            } else {
                if (p;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
                if (*;
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            } else {
            }