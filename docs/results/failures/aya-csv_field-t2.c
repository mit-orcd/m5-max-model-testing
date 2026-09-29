#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int i, j, in_quotes = 0;
    const char *p = line;

    if (idx < 0 || idx >= strcspn(line, ","))
        return -1;

    i = 0;
    while (p[i] && i < idx) {
        if (p[i] == '"')
            in_quotes = !in_quotes;
        i++;
    }

    if (!in_quotes) {
        strncpy(out, p + i, cap - 1);
        return strlen(out);
    }

    j = 0;
    while (p[i] && i < idx && j < cap - 1) {
        if (p[i] == '"') {
            if (p[i + 1] == '"')
                i++; // Skip doubled quotes
        } else if (p[i] == '\\') {
            if (p[i + 1] == '\"')
                i++; // Skip escaped quotes
        }
        out[j++] = p[i++];
    }

    out[j] = 0;
    return j;
}