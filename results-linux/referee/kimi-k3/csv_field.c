#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    for (int field = 0; ; field++) {
        int quoted = (*p == '"');
        const char *s = p + quoted;
        const char *q = s;
        const char *end = s;
        int len = 0;
        for (;;) {
            if (quoted) {
                if (*q == 0) { end = q; break; }
                if (*q == '"') {
                    if (q[1] == '"') { len++; q += 2; }
                    else { end = q; q++; break; }
                } else { len++; q++; }
            } else {
                if (*q == 0 || *q == ',') { end = q; break; }
                len++; q++;
            }
        }
        if (field == idx) {
            int o = 0;
            for (const char *r = s; r < end && o < (int)cap - 1; ) {
                char ch = *r++;
                if (quoted && ch == '"' && *r == '"') r++;
                out[o++] = ch;
            }
            out[o] = 0;
            return len;
        }
        if (*q == 0) break;
        p = q + 1;
    }
    return -1;
}
