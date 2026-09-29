```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    const char *field_start = NULL;
    int in_quotes = 0;

    /* Find the start of the requested field */
    while (1) {
        if (!in_quotes && *p != '"') {
            /* Unquoted field starts at p */
            field_start = p;
            /* Scan to next comma or end of line */
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (current_idx == idx) {
                /* Extract unquoted field */
                size_t len = (size_t)(p - field_start);
                size_t copy_len = len;
                if (copy_len >= cap) {
                    copy_len = cap - 1;
                }
                if (copy_len > 0) {
                    memcpy(out, field_start, copy_len);
                }
                out[copy_len] = '\0';
                return (int)len;
            }
            if (*p == ',') {
                p++;
                current_idx++;
                continue;
            }
            /* End of line */
            if (*p == '\0') {
                /* Check if current_idx matches idx */
                if (current_idx == idx) {
                    /* This case is handled above, but for safety */
                    size_t len = (size_t)(p - field_start);
                    size_t copy_len = len;
                    if (copy_len >= cap) {
                        copy_len = cap - 1;
                    }
                    if (copy_len > 0) {
                        memcpy(out, field_start, copy_len);
                    }
                    out[copy_len] = '\0';
                    return (int)len;
                }
                return -1;
            }
        } else if (*p == '"') {
            /* Quoted field */
            in_quotes = 1;
            field_start = p + 1; /* Start after opening quote */
            p++;
            /* Scan to closing quote */
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        p += 2;
                    } else {
                        /* Closing quote */
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
            if (current_idx == idx) {
                /* Extract quoted field, unescaping doubled quotes */
                size_t out_len = 0;
                const char *src = field_start;
                while (src < p) {
                    if (*src == '"' && *(src + 1) == '"') {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        src += 2;
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *src;
                        }
                        src++;
                    }
                }
                out[out_len] = '\0';
                /* Return unescaped length */
                /* Calculate unescaped length */
                size_t unescaped_len = 0;
                src = field_start;
                while (src < p) {
                    if (*src == '"' && *(src + 1) == '"') {
                        unescaped_len++;
                        src += 2;
                    } else {
                        unescaped_len++;
                        src++;
                    }
                }
                return (int)unescaped_len;
            }
            /* After closing quote, expect comma or end */
            if (*p == ',') {
                p++;
                current_idx++;
                in_quotes = 0;
                continue;
            } else if (*p == '\0') {
                /* End of line */
                if (current_idx == idx) {
                    /* Already handled */
                }
                return -1;
            } else {
                /* Unexpected character after closing quote - malformed, but continue */
                /* Skip to next comma or end */
                while (*p != '\0' && *p != ',') {
                    p++;
                }
                if (*p == ',') {
                    p++;
                    current_idx++;