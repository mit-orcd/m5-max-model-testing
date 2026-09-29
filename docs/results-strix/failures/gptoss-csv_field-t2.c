```c
#include <stddef.h>
#include <limits.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    if (!line) return -1;

    const char *p = line;
    int cur = 0;
    size_t out_len = 0;
    size_t write_cap = (cap > 0) ? cap - 1 : 0;
    char *out_write = out;

    /* Empty line -> one empty field */
    if (*p == '\0') {
        if (idx == 0) {
            if (cap > 0) out[0] = '\0';
            return 0;
        }
        return -1;
    }

    while (*p != '\0') {
        int quoted = (*p == '"');
        if (quoted) p++; /* skip opening quote */

        while (1) {
            if (*p == '\0') break;

            if (quoted) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {          /* escaped quote */
                        if (out_len < write_cap) *out_write++ = '"';