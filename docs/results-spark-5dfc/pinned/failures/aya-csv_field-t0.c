#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int i, j, in_quotes, len = 0;
    char c;

    for (i = 0; line[i] != '\0' && idx-- > 0; i++) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
            while (line[i] == '"') i++;
        }
    }

    if (idx < 0) {
        return -1;
    }

    i--;
    while (line[i] == ' ') i--;

    while (line[i] != '\0' && line[i] != ',' && line[i] != '"') {
        if (len < cap - 1) {
            out[len++] = line[i++];
        } else {
            return len;
        }
    }

    out[len] = '\0';
    return len;
}