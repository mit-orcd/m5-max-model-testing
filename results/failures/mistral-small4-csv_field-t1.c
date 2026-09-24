#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *ptr = line;
    const char *field_start = NULL;
    int field_count = 0;
    int in_quotes = 0;
    int quote_count = 0;
    size_t out_len = 0;

    if (line == NULL || out == NULL || cap == 0) return -1;

    while (*ptr) {
        if (in_quotes) {
            if (*ptr == '"') {
                quote_count++;
            } else {
                quote_count = 0;
            }
        } else {
            if (*ptr == '"') {
                in_quotes = 1;
                quote_count = 1;
                field_start = ptr + 1;
                if (field_count == idx) {
                    if (out_len < cap - 1) {
                        out[out_len++] = *ptr;
                    }
                }
            } else if (*ptr == ',') {
                in_quotes = 0;
                if (field_count == idx) {
                    if (field_start != NULL) {
                        field_start = line;
                        in_quotes = 0;
                        quote_count = 0;
                    }
                    if (out_len < cap - 1) {
                        out[out_len] = '\0';
                    } else {
                        out[cap - 1] = '\0';
                    }
                    return (int)out_len;
                }
                field_count++;
                field_start = NULL;
            } else {
                if (field_count == idx && field_start != NULL) {
                    if (out_len < cap - 1) {
                        out[out_len++] = *ptr;
                    }
                }
            }
        }
        if (field_count == idx && field_start == NULL && !in_quotes) {
            field_start = ptr;
        }
        ptr++;
    }

    if (field_start != NULL) {
        if (field_count == idx) {
            if (out_len < cap - 1) {
                out[out_len] = '\0';
            } else {
                out[cap - 1] = '\0';
            }
            return (int)out_len;
        }
        field_count++;
    }

    return -1;
}