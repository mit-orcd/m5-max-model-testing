#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int i, j, in_quotes, len = 0;
    char c;

    for (i = 0; line[i] != '\0' && idx-- > 0; i++) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
            while (line[i] == '"') i++;  // skip doubled quotes
        }
    }

    if (idx < 0) {
        return -1;  // idx out of range
    }

    i--;  // i is now the start of the field

    while (line[i] != '\0' && len < cap - 1) {
        c = line[i];
        if (in_quotes) {
            if (c == '"') {
                if (line[i + 1] == '"') {
                    out[len++] = '"';
                    i++;  // skip doubled quotes
                } else {
                    out[len++] = c;
                }
            } else {
                out[len++] = c;
            }
        } else {
            if (c == ',' || c == '\r' || c == '\n') {
                break;
            }
            out[len++] = c;
        }
        i++;
        in_quotes = 0;  // assume not in quotes
    }

    out[len] = '\0';  // NUL-terminate

    return len;
}