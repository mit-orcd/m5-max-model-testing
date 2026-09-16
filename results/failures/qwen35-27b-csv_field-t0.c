```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    int in_quotes = 0;
    int field_start = 0;
    int field_end = 0;
    int found = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            found = 1;
            field_start = (int)(p - line);
            field_end = field_start;
        }

        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (found && out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = 0;
                    p++;
                    continue;
                }
            } else {
                if (found && out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
                continue;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
                continue;
            } else if (*p == ',') {
                // End of field
                if (current_idx == idx) {
                    field_end = (int)(p - line);
                }
                current_idx++;
                p++;
                continue;
            } else {
                if (found && out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
                continue;
            }
        }
    }

    // Handle the last field if we reached the end of the string
    if (current_idx == idx) {
        found = 1;
        field_end = (int)(p - line);
    }

    if (!found) {
        return -1;
    }

    // Re-scan the specific field to handle truncation correctly
    // The previous loop filled 'out' but we need to ensure we return the correct length
    // and handle the case where the field was longer than cap-1.
    // We need to re-parse the specific field to get the exact unescaped length.
    
    const char *start = line + field_start;
    const char *end = line + field_end;
    size_t len = 0;
    int q = 0;
    const char *c = start;

    while (c < end) {
        if (q) {
            if (*c == '"') {
                if (*(c + 1) == '"') {
                    len++;
                    c += 2;
                    continue;
                } else {
                    q = 0;
                    c++;
                    continue;
                }
            } else {
                len++;
                c++;
                continue;
            }
        } else {
            if (*c == '"') {
                q = 1;
                c++;
                continue;
            } else {
                len++;
                c++;
                continue;
            }
        }
    }

    // Copy again to handle truncation properly
    out_len = 0;
    c = start;
    q = 0;
    while (c < end) {
        if (q) {
            if (*c == '"') {
                if (*(c + 1) == '"') {
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    c += 2;
                    continue;
                } else {
                    q = 0;
                    c++;
                    continue;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *c;
                }
                c++;
                continue;
            }
        } else {
            if (*c == '"') {
                q = 1;
                c++;
                continue;
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *c;
                }