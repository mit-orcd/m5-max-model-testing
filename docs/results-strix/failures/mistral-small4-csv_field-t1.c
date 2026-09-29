#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    int field_count = 0;
    bool in_quotes = false;
    const char *start = line;
    const char *end = line;
    size_t remaining = cap - 1;
    size_t out_len = 0;

    while (*end != '\0') {
        if (in_quotes) {
            if (*end == '"') {
                if (*(end + 1) == '"') {
                    if (remaining > 0) {
                        out[out_len++] = '"';
                        remaining--;
                    }
                    end += 2;
                } else {
                    in_quotes = false;
                    end++;
                }
            } else {
                if (remaining > 0) {
                    out[out_len++] = *end;
                    remaining--;
                }
                end++;
            }
        } else {
            if (*end == ',') {
                if (field_count == idx) {
                    if (out_len == 0) {
                        out[0] = '\0';
                        return 0;
                    }
                    out[out_len] = '\0';
                    return out_len;
                }
                field_count++;
                out_len = 0;
                remaining = cap - 1;
                start = end + 1;
                end++;
            } else if (*end == '"') {
                in_quotes = true;
                end++;
            } else {
                if (field_count == idx) {
                    if (remaining > 0) {
                        out[out_len++] = *end;
                        remaining--;
                    }
                    end++;
                } else {
                    end++;
                }
            }
        }
    }

    if (field_count == idx) {
        size_t len = end - start;
        if (in_quotes) len = -1;
        else if (len > cap - 1) len = cap - 1;
        memcpy(out, start, len);
        out[len] = '\0';
        return len;
    }

    return -1;
}