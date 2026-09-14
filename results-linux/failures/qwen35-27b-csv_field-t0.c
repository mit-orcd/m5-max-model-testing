#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    bool in_quotes = false;
    bool field_started = false;
    size_t out_len = 0;

    while (*p != '\0') {
        char c = *p;

        if (!in_quotes) {
            if (c == '"') {
                in_quotes = true;
                field_started = true;
                p++;
                continue;
            } else if (c == ',') {
                if (field_started) {
                    current_idx++;
                    field_started = false;
                }
                if (current_idx == idx) {
                    // Found the target field, but it's empty
                    if (cap > 0) out[0] = '\0';
                    return 0;
                }
                p++;
                continue;
            } else {
                field_started = true;
            }
        }

        if (current_idx == idx) {
            if (in_quotes) {
                if (c == '"') {
                    // Check for escaped quote ""
                    if (*(p + 1) == '"') {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                        continue;
                    } else {
                        // End of quoted field
                        in_quotes = false;
                        p++;
                        // Check if followed by comma or end of line
                        if (*p == ',' || *p == '\0') {
                            if (out_len < cap - 1) {
                                out[out_len] = '\0';
                            }
                            return (int)out_len;
                        }
                        // RFC 4180: quotes must be followed by comma or end of line
                        // If not, it's malformed, but we treat as end of field for robustness
                        if (out_len < cap - 1) {
                            out[out_len] = '\0';
                        }
                        return (int)out_len;
                    }
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                    p++;
                    continue;
                }
            } else {
                // Unquoted field
                if (c == ',') {
                    if (out_len < cap - 1) {
                        out[out_len] = '\0';
                    }
                    return (int)out_len;
                } else if (c == '\0') {
                    if (out_len < cap - 1) {
                        out[out_len] = '\0';
                    }
                    return (int)out_len;
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                    p++;
                    continue;
                }
            }
        } else {
            // Skipping fields before the target index
            if (in_quotes) {
                if (c == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    } else {
                        in_quotes = false;
                        p++;
                        continue;
                    }
                } else {
                    p++;
                    continue;
                }
            } else {
                if (c == ',') {
                    current_idx++;
                    field_started = false;
                    p++;
                    continue;
                } else if (c == '\0') {
                    break;
                } else {
                    p++;
                    continue;
                }
            }
        }
    }

    // End of line reached
    if (current_idx == idx) {
        if (out_len < cap - 1) {
            out[out_len] = '\0';
        }
        return (int)out_len;
    }

    return -1;
}