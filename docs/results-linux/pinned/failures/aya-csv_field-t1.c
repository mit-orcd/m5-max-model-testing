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
        if (!*p)
            return -1;
        p++;
    }

    in_quotes = *p == '"';
    if (in_quotes)
        p++;

    for (i = 0; *p && i < cap - 1; i++) {
        if (in_quotes && *p == '"') {
            if (p[1] == '"')
                out[i++] = '"';
            p += 2;
        } else if (*p != ',' && *p != '\r' && *p != '\n') {
            out[i++] = *p;
            p++;
        }
    }

    out[i] = '\0';
    return strlen(out);
}