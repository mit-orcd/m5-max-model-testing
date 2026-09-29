#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || idx >= (int)strcspn(line, ","))
        return -1;
    size_t len = 0;
    bool in_quotes = false;
    char c;
    while ((c = line[idx]) != 0) {
        if (c == '"') {
            in_quotes = !in_quotes;
            if (idx++ == 0 || line[idx - 1] != '"')
                continue;
        }
        if (in_quotes && c == '\\')
            idx++; // skip escaped char
        if (len < cap - 1)
            out[len++] = c;
        if (c == ',' && !in_quotes)
            break;
        idx++;
    }
    out[len] = 0;
    return len;
}