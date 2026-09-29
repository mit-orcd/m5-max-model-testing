#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

static bool csv_unescape(const char *in, char *out, size_t cap) {
    size_t len = 0;
    bool in_quote = false;
    while (*in) {
        if (in_quote) {
            if (*in == '"') {
                in++;
                if (*in == '"') {
                    if (len < cap) out[len++] = '"';
                    in++;
                } else {
                    in_quote = false;
                }
            } else {
                if (len < cap) out[len++] = *in;
                in++;
            }
        } else {
            if (*in == '"') {
                in_quote = true;
                in++;
            } else {
                if (len < cap) out[len++] = *in;
                in++;
            }
        }
    }
    if (in_quote) return false;
    if (len < cap) out[len] = '\0';
    return true;
}

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field = -1;
    bool in_quote = false;
    const char *start = line;
    while (*line) {
        if (in_quote) {
            if (*line == '"') {
                if (line[1] == '"') {
                    line += 2;
                } else {
                    in_quote = false;
                    line++;
                }
            } else {
                line++;
            }
        } else {
            if (*line == '"') {
                in_quote = true;
                line++;
            } else if (*line == ',') {
                if (field == idx) {
                    const char *end = line;
                    if (csv_unescape(start, out, cap)) {
                        return end - start - (start[0] == '"' ? 2 : 0);
                    }
                    return -1;
                }
                field++;
                start = line + 1;
                line++;
            } else {
                line++;
            }
        }
    }
    if (in_quote) return -1;
    if (field == idx) {
        if (csv_unescape(start, out, cap)) {
            size_t len = line - start;
            if (start[0] == '"' && line > start + 1 && *(line - 1) == '"') {
                len -= 2;
            }
            return len;
        }
    }
    return -1;
}