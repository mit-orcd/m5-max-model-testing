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
        if (*p)
            p++;
    }

    in_quotes = 0;
    while (*p && (in_quotes || *p != ',')) {
        if (*p == '"')
            in_quotes = !in_quotes;
        else if (in_quotes && *p == '\\') {
            p++;
            if (*p == '\"' || *p == '\\')
                out[len++] = *p++;
        } else
            out[len++] = *p++;
    }

    out[len] = '\0';
    return len;
}