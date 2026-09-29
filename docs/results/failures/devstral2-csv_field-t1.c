#include <string.h>
#include <stddef.h>
#include <stdbool.h>

static bool is_quoted(const char *s) {
    return s[0] == '"';
}

static const char *skip_quoted_field(const char *s, size_t *len) {
    size_t i = 1;
    while (s[i] != '"' || s[i + 1] == '"') {
        if (s[i] == '"' && s[i + 1] == '"') i += 2;
        else i++;
        if (s[i] == '\0') return NULL;
    }
    *len = i;
    return s + i + 1;
}

static const char *skip_unquoted_field(const char *s, size_t *len) {
    size_t i = 0;
    while (s[i] != ',' && s[i] != '\0') i++;
    *len = i;
    return s + i;
}

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        bool quoted = is_quoted(p);
        const char *end;
        size_t field_len;
        if (quoted) {
            end = skip_quoted_field(p + 1, &field_len);
            if (!end) return -1;
            field_len -= 1; // subtract opening quote
        } else {
            end = skip_unquoted_field(p, &field_len);
        }

        if (current_idx == idx) {
            size_t copy_len = (field_len < cap - 1) ? field_len : cap - 1;
            memcpy(out, p + (quoted ? 1 : 0), copy_len);
            out[copy_len] = '\0';

            // Unescape doubled quotes
            for (size_t i = 0; i < copy_len; i++) {
                if (out[i] == '"' && i + 1 < copy_len && out[i + 1] == '"') {
                    memmove(&out[i], &out[i + 1], copy_len - i - 1);
                    copy_len--;
                }
            }

            return copy_len;
        }

        current_idx++;
        p = end;
        if (*p == ',') p++;
    }

    return (current_idx == idx + 1) ? 0 : -1;
}