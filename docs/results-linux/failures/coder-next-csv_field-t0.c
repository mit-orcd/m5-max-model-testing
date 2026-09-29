#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field = 0;
    size_t out_pos = 0;

    while (field <= idx) {
        // Skip leading whitespace (not required by RFC 4180, but common practice)
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '\0') {
            return -1; // Not enough fields
        }

        if (field == idx) {
            // Extract this field
            int in_quotes = 0;
            size_t field_len = 0;

            if (*p == '"') {
                in_quotes = 1;
                p++;
            }

            while (*p != '\0') {
                if (in_quotes) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            // Escaped quote
                            if (out_pos < cap - 1) {
                                out[out_pos++] = '"';
                            }
                            field_len++;
                            p += 2;
                        } else {
                            // End of quoted field
                            p++;
                            // Skip trailing whitespace and comma
                            while (*p == ' ' || *p == '\t') {
                                p++;
                            }
                            if (*p == ',') {
                                p++;
                            }
                            break;
                        }
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        field_len++;
                        p++;
                    }
                } else {
                    if (*p == ',') {
                        p++;
                        break;
                    } else if (*p == '"') {
                        // Unquoted field containing a quote is invalid per RFC 4180,
                        // but we'll treat it as part of the field
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        field_len++;
                        p++;
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        field_len++;
                        p++;
                    }
                }
            }

            out[out_pos] = '\0';
            return (int)field_len;
        }

        // Skip current field
        int in_quotes = 0;
        if (*p == '"') {
            in_quotes = 1;
            p++;
        }

        while (*p != '\0') {
            if (in_quotes) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                    } else {
                        p++;
                        in_quotes = 0;
                    }
                } else {
                    p++;
                }
            } else {
                if (*p == ',') {
                    p++;
                    break;
                } else {
                    p++;
                }
            }
        }

        // Skip trailing whitespace and comma
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == ',') {
            p++;
        }

        field++;
    }

    return -1;
}