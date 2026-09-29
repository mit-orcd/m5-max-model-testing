int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = 0;
    const char *p = line;

    /* skip leading whitespace (optional per RFC 4180) */
    while (*p == ' ' || *p == '\t') p++;

    for (int i = 0; i < idx; i++) {
        if (*p == '\0') return -1;

        if (*p == '"') {
            /* quoted field */
            p++;
            while (*p && *p != '"') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        break;
                    }
                } else {
                    p++;
                }
            }
            if (*p != '"') return -1; /* unterminated quote */
            p++; /* skip closing quote */
        } else {
            /* unquoted field */
            while (*p && *p != ',' && *p != '\n' && *p != '\r') p++;
        }

        /* consume trailing CR/LF if present */
        if (*p == '\r') {
            if (*(p + 1) == '\n') p += 2;
            else p++;
        } else if (*p == '\n') {
            p++;
        }

        if (*p == ',') p++;
        else if (*p == '\0') return -1; /* end of line before reaching idx */
    }

    /* now p points to start of field idx */
    if (*p == '"') {
        /* quoted field */
        p++;
        while (*p && *p != '"') {
            if (*p == '"' && *(p + 1) == '"') {
                if (len + 1 < cap) out[len] = '"';
                len++;
                p += 2;
            } else {
                if (len + 1 < cap) out[len] = *p;
                len++;
                p++;
            }
        }
        if (*p != '"') return -1; /* unterminated quote */
        p++; /* skip closing quote */
    } else {
        /* unquoted field */
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            if (len + 1 < cap) out[len] = *p;
            len++;
            p++;
        }
    }

    if (len < cap) out[len] = '\0';
    else if (cap > 0) out[cap - 1] = '\0';

    return (int)len;
}