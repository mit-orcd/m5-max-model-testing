#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (true) {
        bool in_quotes = false;
        size_t written = 0;
        bool field_found = false;

        if (current_idx == idx) {
            field_found = true;
        }

        // Check if we are at the start of a field
        // A field starts at the beginning of the string or after a comma
        // However, we must skip the content of previous fields first.
        
        // This loop logic handles skipping or extracting
        // We need to parse the structure of the current field
        
        // If we are looking for the current idx, we process it.
        // If we are skipping, we just advance p.
        
        // Let's refine: we iterate through fields one by one.
    }
    return -1; // Placeholder for logic structure
}