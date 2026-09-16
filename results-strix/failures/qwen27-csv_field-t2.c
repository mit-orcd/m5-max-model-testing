#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;

    for (;;) {
        /* skip leading whitespace? RFC-4180 does not require it; we do not skip */
        if (*p == '"') {
            /* quoted field */
            p++; /* skip opening quote */
            size_t o = 0;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        /* escaped quote */
                        if (o + 1 < cap) out[o++] = '"';
                        p += 2;
                    } else {
                        /* closing quote */
                        p++;
                        break;
                    }
                } else {
                    if (o + 1 < cap) out[o++] = *p;
                    p++;
                }
            }
            /* after closing quote, expect comma or end of line */
            if (*p == ',') {
                p++;
                if (o + 1 < cap) out[o] = '\0';
                else if (cap > 0) out[cap - 1] = '\0';
                if (field_idx == idx) return (int)o;
                field_idx++;
            } else {
                /* end of line (or unexpected char); treat as last field */
                if (o + 1 < cap) out[o] = '\0';
                else if (cap > 0) out[cap - 1] = '\0';
                if (field_idx == idx) return (int)o;
                return -1;
            }
        } else {
            /* unquoted field */
            size_t o = 0;
            while (*p && *p != ',') {
                if (o + 1 < cap) out[o++] = *p;
                p++;
            }
            if (*p == ',') {
                p++;
                if (o + 1 < cap) out[o] = '\0';
                else if (cap > 0) out[cap - 1] = '\0';
                if (field_idx == idx) return (int)o;
                field_idx++;
            } else {
                /* end of line */
                if (o + 1 < cap) out[o] = '\0';
                else if (cap > 0) out[cap - 1] = '\0';
                if (field_idx == idx) return (int)o;
                return -1;
            }
        }
    }
}