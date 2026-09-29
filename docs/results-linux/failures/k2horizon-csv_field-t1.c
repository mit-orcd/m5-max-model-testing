#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int cur = 0;
    int in_quotes = 0;
    size_t out_len = 0;
    int escaped_quote = 0;

    if (line == NULL || out == NULL) return -1;

    while (*line) {
        char c = *line;

        if (in_quotes) {
            if (c == '"') {
                if (escaped_quote) {
                    if (out_len + 1 < cap) out[out_len++] = '"';
                    escaped_quote = 0;
                } else {
                    escaped_quote = 1;
                }
            } else {
                if (escaped_quote) {
                    if (out_len + 1 < cap) out[out_len++] = '"';
                    escaped_quote = 0;
                }
                if (c == ',') {
                    if (out_len + 1 < cap) out[out_len++] = ',';
                } else {
                    if (out_len + 1 < cap) out[out_len++] = c;
                }
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                escaped_quote = 0;
            } else if (c == ',') {
                if (cur == idx) {
                    if (out_len + 1 < cap) out[out_len++] = '\0';
                    return (int)out_len;
                }
                cur++;
            } else {
                if (escaped_quote) {
                    if (out_len + 1 < cap) out[out_len++] = '"';
                    escaped_quote = 0;
                }
                if (out_len + 1 < cap) out[out_len++] = c;
            }
        }
        line++;
    }

    if (in_quotes && escaped_quote) {
        if (out_len + 1 < cap) out[out_len++] = '"';
        escaped_quote = 0;
    }

    if (cur == idx) {
        if (out_len + 1 < cap) out[out_len++] = '\0';
        return (int)out_len;
    }

    return -1;
}