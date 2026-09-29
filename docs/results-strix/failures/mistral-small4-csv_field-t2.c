#include <stddef.h>
#include <ctype.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t pos = 0;
    size_t field_start = 0;
    int field_idx = -1;
    char *pout = out;
    const char *pline = line;
    size_t remaining = cap;

    if (cap == 0) return -1;

    while (pos < cap && field_idx < idx) {
        if (*pline == '\0') break;

        if (field_idx == idx - 1) {
            if (*pline == ',') {
                field_idx = idx;
                break;
            }
            if (*pline == '"') {
                pline++;
                field_idx = idx;
                break;
            }
        }

        if (*pline == '"') {
            pline++;
            field_idx++;
            if (field_idx >= idx) {
                while (pos < cap - 1 && *pline != '\0') {
                    if (*pline == '"') {
                        pline++;
                        if (*pline == '"') {
                            *pout++ = '"';
                            pos++;
                            pline++;
                        } else {
                            if (field_idx == idx) break;
                            field_idx++;
                            if (field_idx < idx) {
                                while (isspace((unsigned char)*pline)) pline++;
                                if (*pline == ',') pline++;
                            }
                            break;
                        }
                    } else {
                        *pout++ = *pline++;
                        pos++;
                    }
                }
                if (*pline != '\0') {
                    while (isspace((unsigned char)*pline) && field_idx < idx) {
                        pline++;
                        if (field_idx == idx - 1 && *pline == ',') {
                            field_idx = idx;
                            break;
                        }
                    }
                }
            } else {
                if (field_idx == idx) {
                    field_start = pos;
                    while (pos < cap - 1 && *pline != '\0' && *pline != '\n') {
                        if (*pline == '"') {
                            pline++;
                            if (*pline != '"') {
                                break;
                            }
                            *pout++ = '"';
                            pos++;
                            pline++;
                        } else {
                            *pout++ = *pline++;
                            pos++;
                        }
                    }
                } else {
                    while (pos < cap - 1 && *pline != '\0' && *pline != '\n' && *pline != '"') {
                        pline++;
                    }
                    if (*pline == '"') {
                        pline++;
                        while (isspace((unsigned char)*pline) && *pline != '\0') {
                            pline++;
                        }
                        if (*pline == ',') pline++;
                    }
                }
            }
        } else {
            while (pos < cap - 1 && *pline != '\0' && *pline != '\n' && *pline != ',' && *pline != '"') {
                *pout++ = *pline++;
                pos++;
            }
            if (*pline == ',') {
                field_idx++;
                pline++;
            }
        }

        if (field_idx == idx) {
            size_t len = pos - field_start;
            if (len < remaining) {
                *pout = '\0';
                return len;
            } else {
                *pout = '\0';
                return cap - 1;
            }
        }
    }

    return -1;
}