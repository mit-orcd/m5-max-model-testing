#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;
    size_t len = 0;
    size_t out_pos = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't require it; we don't skip. */
        if (*p == '"') {
            /* Quoted field */
            p++; /* skip opening quote */
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        if (out_pos < cap - 1) {
                            out[out_pos++] = '"';
                            len++;
                        }
                        p += 2;
                    } else {
                        /* Closing quote */
                        p++;
                        break;
                    }
                } else {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                        len++;
                    }
                    p++;
                }
            }
            /* After closing quote, expect comma or end of line */
            if (*p == ',') {
                p++;
            } else {
                /* End of line or invalid; we're done with this field */
                break;
            }
        } else {
            /* Unquoted field */
            while (*p != '\0' && *p != ',') {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                    len++;
                }
                p++;
            }
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        }

        out_pos++; /* This is not correct; we should NUL terminate at the end */
        /* Actually, we need to check if we've reached the desired field */
        if (field_idx == idx) {
            break;
        }
        field_idx++;
    }

    /* Wait, the logic above is flawed. Let me restructure. */

    /* Let me rewrite properly */
    p = line;
    field_idx = 0;
    len = 0;
    out_pos = 0;

    while (1) {
        if (*p == '"') {
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = '"';
                            len++;
                        }
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                        len++;
                    }
                    p++;
                }
            }
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        } else {
            while (*p != '\0' && *p != ',') {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                    len++;
                }
                p++;
            }
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        }

        if (field_idx == idx) {
            out[out_pos] = '\0';
            return (int)len;
        }

        field_idx++;
        /* Reset for next field */
        len = 0;
        out_pos = 0;
    }

    /* If we exited the loop without finding idx, check if we just finished field idx */
    /* Actually the break happens after processing a field. Let's check: */
    /* The loop processes a field, then checks if field_idx == idx. If not, increments field_idx and continues. */
    /* If *p is not ',' (end of line), we break after processing the current field. */
    /* So after the loop, if we broke because of end-of-line, we need to check if field_idx == idx */

    if (field_idx == idx) {
        out[out_pos] = '\0';
        return (int)len;
    }

    return -1;
}