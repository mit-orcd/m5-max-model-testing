#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;

    for (;;) {
        /* Skip whitespace? RFC 4180 doesn't allow leading/trailing spaces in unquoted fields,
           but we'll just process as-is. */
        
        /* Parse current field */
        const char *field_start;
        const char *field_end;
        int is_quoted = 0;

        if (*p == '"') {
            is_quoted = 1;
            p++; /* skip opening quote */
            field_start = p;
            
            /* Find closing quote (not followed by another quote) */
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; /* doubled quote, skip both */
                    } else {
                        p++; /* closing quote */
                        break;
                    }
                } else {
                    p++;
                }
            }
            field_end = p; /* points to closing quote or NUL */
            if (*p == '"') {
                p++; /* skip closing quote */
            }
        } else {
            field_start = p;
            while (*p != '\0' && *p != ',') {
                p++;
            }
            field_end = p;
        }

        /* Check if this is the desired field */
        if (field_idx == idx) {
            /* Extract unescaped content */
            size_t out_len = 0;
            const char *src = field_start;
            const char *src_end = field_end;
            
            if (is_quoted) {
                /* src points to content after opening quote, src_end points to closing quote */
                while (src < src_end && out_len < cap - 1) {
                    if (*src == '"' && *(src + 1) == '"') {
                        out[out_len++] = '"';
                        src += 2;
                    } else {
                        out[out_len++] = *src;
                        src++;
                    }
                }
                out[out_len] = '\0';
            } else {
                size_t len = (size_t)(src_end - src);
                if (len > cap - 1) {
                    len = cap - 1;
                }
                memcpy(out, src, len);
                out[len] = '\0';
                out_len = len;
            }
            
            /* Return the unescaped length (before truncation) */
            /* Compute actual unescaped length */
            size_t actual_len = 0;
            if (is_quoted) {
                const char *s = field_start;
                const char *e = field_end;
                while (s < e) {
                    if (*s == '"' && *(s + 1) == '"') {
                        actual_len++;
                        s += 2;
                    } else {
                        actual_len++;
                        s++;
                    }
                }
            } else {
                actual_len = (size_t)(field_end - field_start);
            }
            
            return (int)actual_len;
        }

        /* Move to next field */
        field_idx++;
        
        if (*p == ',') {
            p++; /* skip comma */
            continue;
        }
        
        /* End of line without comma */
        break;
    }

    /* idx out of range */
    return -1;
}