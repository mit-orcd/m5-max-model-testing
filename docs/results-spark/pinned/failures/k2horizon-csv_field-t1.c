```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;
    if (idx < 0) return -1;

    const char *p = line;
    int field = 0;
    size_t len = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            /* quoted field */
            p++;
            while (*p != '\0' && *p != '"') {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                    } else {
                        p++; /* closing quote */
                        break;
                    }
                } else {
                    p++;
                }
            }
            /* skip to end of field */
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    /* extract field idx */
    p = line;
    field = 0;
    len = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            p++;
            while (*p != '\0' && *p != '"') {
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
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    /* copy field */
    p = line;
    field = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            p++;
            while (*p != '\0' && *p != '"') {
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
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    /* copy field content */
    p = line;
    field = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            p++;
            while (*p != '\0' && *p != '"') {
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
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    /* copy field content */
    p = line;
    field = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            p++;
            while (*p != '\0' && *p != '"') {
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
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    /* copy field content */
    p = line;
    field = 0;

    while (*p != '\0') {
        if (field == idx) break;

        if (*p == '"') {
            p++;
            while (*p != '\0' && *p != '"') {
                if (*p == '"') {
                    if (p[1