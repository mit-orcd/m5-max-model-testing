#include <string.h>
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    
    const char *p = line;
    int field = 0;
    size_t pos = 0;
    
    while (field < idx) {
        if (!*p) return -1;
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            while (*p && *p != ',') p++;
        }
        if (*p == ',') p++;
        field++;
    }
    
    if (!*p) return -1;
    
    if (*p == '"') {
        p++;
        while (*p && pos < cap - 1) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (pos < cap - 1) {
                        out[pos++] = '"';
                    }
                    p += 2;
                } else {
                    p++;
                    break;
                }
            } else {
                out[pos++] = *p++;
            }
        }
    } else {
        while (*p && *p != ',' && pos < cap - 1) {
            out[pos++] = *p++;
        }
    }
    
    out[pos] = '\0';
    
    // Calculate unescaped length
    size_t len = 0;
    p = line;
    for (int i = 0; i < idx; i++) {
        if (!*p) return -1;
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                        len++;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    p++;
                    len++;
                }
            }
        } else {
            while (*p && *p != ',') {
                p++;
                len++;
            }
        }
        if (*p == ',') p++;
    }
    
    if (*p == '"') {
        p++;
        while (*p) {
            if (*p == '"') {
                if (p[1] == '"') {
                    p += 2;
                    len++;
                } else {
                    p++;
                    break;
                }
            } else {
                p++;
                len++;
            }
        }
    } else {
        while (*p && *p != ',') {
            p++;
            len++;
        }
    }
    
    return (int)len;
}