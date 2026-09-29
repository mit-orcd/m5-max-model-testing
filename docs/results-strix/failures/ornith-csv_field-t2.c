int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t i = 0;
    size_t len = 0;
    int in_quotes = 0;

    if (cap == 0) {
        return -1;
    }

    while (line[i] != '\0') {
        if (idx == 0) {
            if (line[i] == '"') {
                in_quotes = 1;
                i++;
                while (line[i] != '\0') {
                    if (line[i] == '"') {
                        if (line[i + 1] == '"') {
                            if (len < cap - 1) {
                                out[len] = '"';
                            }
                            len += 2;
                            i += 2;
                        } else {
                            i++;
                            while (line[i] != '\0' && line[i] != '"') {
                                if (line[i] == ',') {
                                    if (cap > 0) {
                                        out[len] = '\0';
                                    }
                                    return (int)len;
                                }
                                if (len < cap - 1) {
                                    out[len] = line[i];
                                }
                                len++;
                                i++;
                            }
                            if (line[i] == '"') {
                                i++;
                            }
                            if (line[i] == ',') {
                                if (len < cap - 1) {
                                    out[len] = '\0';
                                }
                                return (int)len;
                            } else if (line[i] == '\0') {
                                if (len < cap - 1) {
                                    out[len] = '\0';
                                }
                                return (int)len;
                            }
                        }
                    } else {
                        if (line[i] == ',') {
                            if (len < cap - 1) {
                                out[len] = '\0';
                            }
                            return (int)len;
                        }
                        if (len < cap - 1) {
                            out[len] = line[i];
                        }
                        len++;
                        i++;
                    }
                }
            } else {
                while (line[i] != '\0' && line[i] != ',') {
                    if (len < cap - 1) {
                        out[len] = line[i];
                    }
                    len++;
                    i++;
                }
                if (len < cap - 1) {
                    out[len] = '\0';
                }
                return (int)len;
            }
        } else {
            while (line[i] != '\0' && line[i] != ',') {
                i++;
            }
            if (line[i] == ',') {
                i++;
                idx--;
            }
        }
    }

    if (idx == 0) {
        if (len < cap - 1) {
            out[len] = '\0';
        }
        return (int)len;
    }

    return -1;
}