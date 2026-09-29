#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    bool in_quotes = false;
    bool field_started = false;

    while (*p != '\0') {
        char c = *p;

        if (current_idx == idx) {
            field_started = true;
        }

        if (in_quotes) {
            if (c == '"') {
                if (*(p + 1) == '"') {
                    if (field_started) {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                    }
                    p++;
                } else {
                    in_quotes = false;
                }
            } else {
                if (field_started) {
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                }
            }
        } else {
            if (c == '"') {
                in_quotes = true;
                field_started = true;
            } else if (c == ',') {
                if (field_started) {
                    current_idx++;
                    field_started = false;
                } else {
                    // Empty field before comma
                    current_idx++;
                }
                out_len = 0;
                continue;
            } else {
                if (field_started) {
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                }
            }
        }

        p++;

        // Handle end of line
        if (*p == '\0') {
            if (!in_quotes) {
                current_idx++;
            }
        }
    }

    if (current_idx <= idx) {
        return -1;
    }

    out[out_len] = '\0';
    return (int)out_len;
}