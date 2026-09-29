#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    int field_idx = 0;
    const char *p = line;
    int start_pos = 0;
    int end_pos = 0;
    int in_quotes = 0;
    int escaped = 0;
    
    while (*p) {
        if (!in_quotes && *p == ',') {
            if (field_idx == idx) {
                break;
            }
            field_idx++;
            start_pos = p - line + 1;
        } else if (*p == '"') {
            if (escaped) {
                escaped = 0;
            } else if (in_quotes) {
                if (*(p+1) == '"') {
                    escaped = 1;
                } else {
                    in_quotes = 0;
                }
            } else {
                in_quote = 1;
            }
        } else if (in_quotes && *p == '"') {
            escaped = 0;
        }
        p++;
    }
    
    if (field_idx != idx) {
        return -1;
    }
    
    const char *start = line + start_pos;
    const char *p = start;
    char *out_p = out;
    size_t out_len = 0;
    int in_quotes = 0;
    int escaped = 0;
    
    while (*p) {
        if (!in_quotes && *p == ',') {
            break;
        }
        
        if (*p == '"' && !escaped) {
            in_quotes = !in_quotes;
        } else if (escaped && *p == '"') {
            escaped = 0;
        } else if (in_quotes) {
            if (*p == '"') {
                escaped = 1;
            } else {
                escaped = 0;
            }
        }
        
        if (out_len < cap - 1) {
            *out_p++ = *p;
            out_len++;
        }
        p++;
    }
    
    *out_p = '\0';
    
    return out_len;
}