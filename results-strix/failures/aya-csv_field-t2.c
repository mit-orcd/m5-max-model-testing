#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || idx >= strlen(line) - 1) {
        return -1;
    }
    const char *field_start = &line[idx + 1];
    size_t len = 0;
    while (*field_start && len < cap - 1) {
        if (*field_start == '"') {
            field_start++;
            while (*field_start && *field_start != '"') {
                out[len++] = *field_start++;
            }
            if (*field_start == '"') {
                field_start++;
                if (*field_start == '"') {
                    out[len++] = *field_start++;
                }
            }
        } else {
            while (*field_start && *field_start != ',' && len < cap - 1) {
                out[len++] = *field_start++;
            }
        }
    }
    out[len] = '\0';
    return len;
}