#include <string.h>
#include <stdlib.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    
    int field_count = 0;
    const char *p = line;
    
    // Skip to the desired field
    while (*p && field_count < idx) {
        if (*p == '"') {
            // Quoted field
            p++;
            while (*p && *p != '"') p++;
            if (*p) p++; // Skip closing quote
            if (*p == '"') p++; // Skip doubled quote
        } else {
            // Unquoted field
            while (*p && *p != ',') p++;
        }
        if (*p == ',') p++;
        field_count++;
    }
    
    if (field_count != idx) return -1;
    
    if (!*p) return -1;
    
    size_t out_len = 0;
    
    if (*p == '"') {
        // Quoted field
        p++;
        while (*p && *p != '"') {
            if (out_len < cap - 1) out[out_len++] = *p;
            p++;
        }
        if (*p == '"') p++; // Skip closing quote
        // Handle doubled quotes
        while (*p == '"') {
            if (out_len < cap - 1) out[out_len++] = '"';
            p++;
            if (*p == '"') p++; // Skip next quote
        }
        if (*p == '"') p++; // Skip closing quote
    } else {
        // Unquoted field
        while (*p && *p != ',') {
            if (out_len < cap - 1) out[out_len++] = *p;
            p++;
        }
    }
    
    out[out_len] = '\0';
    return (int)out_len;
}