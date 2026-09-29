#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int i, j, in_quotes, len = 0;
    const char *p = line;

    if (idx < 0 || idx >= strcspn(line, ","))
        return -1;

    for (i = 0; i < idx; i++) {
        while (*p && *p != ',' && *p != '"')
            p++;
        if (*p == ',')
            p++;
        else if (*p == '"') {
            in_quotes = 1;
            while (*p && *p != '"')
                p++;
            if (*p == '"')
                p++;
        }
    }

    while (*p && *p != ',' && *p != '\r' && *p != '\n') {
        if (in_quotes && *p == '\\') {
            if (p[1] == '\"' || p[1] == '\\') {
                out[len++] = p[1];
                p += 2;
            }
        } else {
            out[len++] = *p;
            p++;
        }
    }

    out[len] = '\0';
    return len;
}