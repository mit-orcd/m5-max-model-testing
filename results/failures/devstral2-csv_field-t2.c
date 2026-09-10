#include <stdlib.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field = 0;
    int len = 0;
    int quote = 0;

    while (*p) {
        if (field == idx) {
            while (*p && (quote || *p != ',')) {
                if (quote) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            p++;
                            if (out && cap) {
                                if (len < cap - 1) {
                                    out[len++] = '"';
                                }
                            }
                        } else {
                            quote = 0;
                        }
                    } else {
                        if (out && cap) {
                            if (len < cap - 1) {
                                out[len++] = *p;
                            }
                        }
                    }
                } else {
                    if (*p == '"') {
                        quote = 1;
                    } else {
                        if (out && cap) {
                            if (len < cap - 1) {
                                out[len++] = *p;
                            }
                        }
                    }
                }
                p++;
            }
            if (out && cap) {
                out[len] = '\0';
            }
            return len;
        }

        if (*p == '"') {
            quote = 1;
            p++;
        } else {
            if (*p == ',') {
                field++;
                p++;
                quote = 0;
            } else {
                p++;
            }
        }
    }

    if (field == idx && !quote) {
        if (out && cap) {
            out[0] = '\0';
        }
        return 0;
    }

    return -1;
}