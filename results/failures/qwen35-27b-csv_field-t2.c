#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    char *dst = out;
    size_t written = 0;
    int field_count = 0;
    int in_quotes = 0;

    if (line == NULL || idx < 0 || (cap == 0 && idx != -1)) {
        return -1;
    }

    while (*p != '\0') {
        in_quotes = 0;

        if (*p == '"') {
            // Handle quoted field
            if (*(p + 1) == '"') {
                p += 2;
                in_quotes = 1;
            } else {
                // Unquoted field that starts with quote or just a quote
                // RFC 4180: If the field contains quotes, it must be enclosed in quotes.
                // If it starts with a quote, it's a quoted field.
                // If it's just a quote followed by something else, it's treated as part of the field content
                // unless we assume strict RFC 4180 where non-quoted fields cannot contain quotes.
                // We will treat a leading quote as the start of a quoted field.
                // If the next char is not a quote, it's an unquoted field starting with a quote char?
                // RFC 4180 says: "If the field contains ... quotes, ... the field MUST be enclosed in quotes."
                // So if we see a quote at the start, we assume it's a quoted field.
                // If the quote is not followed by another quote, we treat the content inside.
                // However, standard parser logic:
                // If we are not in quotes and see a quote, we enter "quoted mode".
                // If we are in quoted mode and see a quote, check next.
                // Let's implement standard state machine.
            }
        }

        // Reset position for the actual field start logic based on state
        const char *field_start = p;
        int current_in_quotes = 0;
        
        // Skip leading quote if present for this field
        if (*p == '"') {
            current_in_quotes = 1;
            p++;
            field_start = p;
        }

        while (*p != '\0') {
            if (current_in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (cap > 1) {
                            *dst++ = '"';
                            written++;
                        }
                        p += 2;
                    } else {
                        // End of quoted field
                        p++;
                        break;
                    }
                } else {
                    if (cap > 1) {
                        *dst++ = *p;
                        written++;
                    }
                    p++;
                }
            } else {
                // Unquoted field
                if (*p == ',') {
                    // End of field
                    p++;
                    break;
                } else {
                    if (cap > 1) {
                        *dst++ = *p;
                        written++;
                    }
                    p++;
                }
            }
        }

        if (field_count == idx) {
            *dst = '\0';
            return (int)written;
        }

        field_count++;
        
        if (*p == ',') {
            p++;
        } else if (*p == '\0') {
            // End of line
            break;
        } else {
            // Malformed or error, but we continue to skip
            // If we hit end of line without comma, we've processed the last field
            break;
        }
    }

    // If we haven't found the field yet
    if (idx == field_count && idx == 0) {
        // Edge case: empty line, idx 0
        if (cap > 0) *out = '\0';
        return 0;
    }

    return -1;
}