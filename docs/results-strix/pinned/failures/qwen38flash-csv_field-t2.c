```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
        if (out && cap > 0) out[0] = '\0';
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    int in_quotes = 0;
    int field_started = 0;

    /* Skip to the target field */
    while (current_idx < idx) {
        if (*p == '\0') {
            /* Out of range */
            if (out && cap > 0) out[0] = '\0';
            return -1;
        }

        if (*p == '"') {
            /* Quoted field */
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        p += 2;
                    } else {
                        /* End of quoted field */
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
            /* After closing quote, expect comma or end of line */
            if (*p == ',') {
                p++;
                current_idx++;
            } else if (*p == '\0') {
                /* End of line, no more fields */
                if (out && cap > 0) out[0] = '\0';
                return -1;
            } else {
                /* Invalid format, but treat as end */
                if (out && cap > 0) out[0] = '\0';
                return -1;
            }
        } else {
            /* Unquoted field */
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ',') {
                p++;
                current_idx++;
            } else if (*p == '\0') {
                /* End of line */
                if (out && cap > 0) out[0] = '\0';
                return -1;
            } else {
                if (out && cap > 0) out[0] = '\0';
                return -1;
            }
        }
    }

    /* Now p points to the start of the target field */
    /* Extract the field */
    if (*p == '"') {
        /* Quoted field */
        p++;
        while (*p != '\0') {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    /* Escaped quote */
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                } else {
                    /* End of quoted field */
                    p++;
                    break;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        }
    } else {
        /* Unquoted field */
        while (*p != '\0' && *p != ',') {
            if (out_len < cap - 1) {
                out[out_len++] = *p;
            }
            p++;
        }
    }

    out[out_len] = '\0';

    /* Calculate the full unescaped length (not truncated) */
    /* We need to re-scan to get the true length */
    /* Let's re-scan from the start of this field */
    /* Actually, let's compute it during extraction properly */
    /* Re-do: compute true length */

    /* Reset and compute true length */
    size_t true_len = 0;
    const char *q = p;
    /* We need to find where the field started */
    /* Let's just re-parse from the beginning for the target field */

    /* Simpler: re-parse to get true length */
    /* Go back to start */
    p = line;
    current_idx = 0;
    while (current_idx < idx) {
        if (*p == '"') {
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;