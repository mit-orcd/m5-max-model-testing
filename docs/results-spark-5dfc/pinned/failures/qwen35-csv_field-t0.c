#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    const char *p = line;
    const char *field_start = NULL;
    int current_idx = 0;
    int in_quotes = 0;
    int field_len = 0;
    int i = 0;

    while (*p) {
        if (current_idx == idx) {
            field_start = p;
            field_len = 0;
        }

        if (current_idx == idx && *p == '"') {
            if (in_quotes) {
                if (*(p + 1) == '"') {
                    p++;
                    field_len++;
                } else {
                    in_quotes = 0;
                }
            } else {
                in_quotes = 1;
            }
        } else if (current_idx == idx && *p == ',' && !in_quotes) {
            break;
        } else if (current_idx == idx) {
            field_len++;
        }

        if (current_idx == idx && *p == ',' && !in_quotes) {
            break;
        }

        if (*p == ',' && !in_quotes) {
            current_idx++;
            if (current_idx > idx) break;
        }
        p++;
    }

    if (current_idx != idx) {
        return -1;
    }

    if (field_start == NULL) {
        return -1;
    }

    size_t copy_len = (size_t)field_len;
    if (copy_len >= cap) {
        copy_len = cap - 1;
    }

    const char *src = field_start;
    char *dst = out;
    int j = 0;
    int k = 0;

    while (k < copy_len && *src) {
        if (*src == '"') {
            if (*(src + 1) == '"') {
                *dst++ = '"';
                src++;
                k++;
            } else {
                src++;
            }
        } else {
            *dst++ = *src;
            k++;
        }
        src++;
    }
    *dst = '\0';

    return (int)copy_len;
}