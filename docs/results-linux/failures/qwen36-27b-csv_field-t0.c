#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            size_t out_len = 0;
            if (*p == '"') {
                /* Quoted field */
                p++; /* skip opening quote */
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
                            p++; /* skip closing quote */
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                }
                /* Skip any trailing characters until comma or end of line */
                while (*p != ',' && *p != '\0') {
                    p++;
                }
                out[out_len] = '\0';
                return (int)out_len;
            } else {
                /* Unquoted field */
                while (*p != ',' && *p != '\0') {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
                out[out_len] = '\0';
                return (int)out_len;
            }
        }

        /* Skip to next field */
        if (*p == '"') {
            /* Quoted field: skip until closing quote */
            p++; /* skip opening quote */
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; /* skip escaped quote */
                    } else {
                        p++; /* skip closing quote */
                        break;
                    }
                } else {
                    p++;
                }
            }
            /* Skip any trailing characters until comma or end of line */
            while (*p != ',' && *p != '\0') {
                p++;
            }
        } else {
            /* Unquoted field: skip until comma or end of line */
            while (*p != ',' && *p != '\0') {
                p++;
            }
        }

        /* Skip the comma if present */
        if (*p == ',') {
            p++;
        }

        current_idx++;
    }

    /* If we reached here, idx was out of range */
    return -1;
}