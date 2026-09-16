#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) return -1;
    size_t out_len = 0;
    int field_idx = 0;
    const char *p = line;
    int in_quotes = 0;
    size_t field_start = 0;

    while (*p && (field_idx <= idx)) {
        if (!in_quotes) {
            if (*p == '"') {
                in_quotes = 1;
                field_start = p - line + 1;
            } else if (*p == ',') {
                if (field_idx == idx) break;
                field_idx++;
                p++;
                continue;
            }
        } else {
            if (*p == '"') {
                if (p[1] == '"') {
                    p++; // skip escaped quote
                } else {
                    in_quotes = 0;
                }
            }
        }
        p++;
        if (!in_quotes && *p == ',') {
            if (field_idx == idx) break;
            field_idx++;
            p++;
        }
    }

    if (field_idx != idx) return -1;

    const char *field_end = p;
    if (in_quotes) {
        while (*p && *p != '"') p++;
        if (*p == '"') {
            field_end = p;
            p++; // consume closing quote
            if (*p == ',') p++; // consume trailing comma if present
        } else {
            field_end = p; // unterminated quote, treat as end
        }
    }

    const char *src = line + field_start;
    size_t len = field_end - src;
    if (len >= cap) len = cap - 1;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == '"' && src[i+1] == '"') {
            out[i] = '"';
            i++; // skip next quote
        } else {
            out[i] = src[i];
        }
    }
    out[len] = '\0';

    // Calculate actual unescaped length
    size_t actual_len = 0;
    const char *q = src;
    while (q < field_end) {
        if (*q == '"' && q[1] == '"') {
            actual_len++;
            q += 2;
        } else {
            actual_len++;
            q++;
        }
    }
    return (int)actual_len;
}