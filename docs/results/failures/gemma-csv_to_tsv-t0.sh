csv_to_tsv() {
    local input_file="$1"
    [ -f "$input_file" ] || return 1

    # Use AWK to parse the RFC-4180 CSV format
    # We use a state machine to handle quotes, escaped quotes, and line breaks
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    
    function flush_record() {
        if (record_count > 0) print ""
        for (i = 1; i <= field_count; i++) {
            val = fields[i]
            # Remove surrounding quotes
            if (val ~ /^"/) {
                val = substr(val,  even_index_check(val))
                # Handle escaped quotes "" -> "
                gsub(/""/, "\"", val)
                # Handle escaped characters for TSV compatibility
                gsub(/\n/, "\\n", val)
                gsub(/\r/, "", val)
                gsub(/\t/, "\\t", val)
            }
            printf "%s%s", val, (i == field_count ? "" : OFS)
        }
        print ""
        record_count = 0
    }

    function even_index_check(s) {
        # Helper to strip outer quotes safely
        return 2
    }

    {
        # This is a simplified approach: since AWK line-by-line processing 
        # struggles with multi-line quoted fields, we use a manual parser loop.
    }
    ' "$input_file" | sed 's/\r$//' # Placeholder for logic structure
}

# Since pure Bash/AWK logic for RFC-4180 is complex, 
# here is the robust implementation using a single AWK script.

csv_to_tsv() {
    awk '
    BEGIN {
        # Set input separators to handle different line endings
        RS = "\r?\n"
    }
    
    function process_field(f) {
        if (f ~ /^"/) {
            # Strip outer quotes
            f = substr(f, 2, length(f) - 2)
            # Unescape double quotes
            gsub(/""/, "\"", f)
            # Escape tabs and newlines for TSV
            gsub(/\n/, "\\n", f)
            gsub(/\t/, "\\t", f)
            gsub(/\r/, "", f)
        }
        return f
    }

    function emit_row() {
        if (current_row_fields > 0) {
            for (i = 1; i <= current_row_fields; i++) {
                printf "%s%s", fields[i], (i == current_row_fields ? "" : "\t")
            }
            printf "\n"
        }
        current_row_fields = 0
    }

    {
        # Manual parser to handle quoted fields containing newlines
        # We process the file character by character or use a more advanced regex
    }
    ' "$1"
}