csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = "\n|\r\n"
        FS = ""
        OFS = ""
        in_quote = 0
        current_field = ""
        current_record = ""
        field_count = 0
    }

    function output_record() {
        if (field_count > 0) {
            for (i = 1; i <= field_count; i++) {
                printf "%s", fields[i]
                if (i < field_count) printf "\t"
            }
            printf "\n"
        }
        # Reset state
        field_count = 0
        delete fields
    }

    {
        # Remove trailing CR if present (handling CRLF)
        gsub(/\r$/, "")
        
        line = $0
        len = length(line)
        
        for (i = 1; i <= len; i++) {
            char = substr(line, i, 1)
            
            if (in_quote) {
                if (char == "\"") {
                    # Check for escaped quote ""
                    if (i < len && substr(line, i+1, 1) == "\"") {
                        current_field = current_field "\""
                        i++ # Skip next quote
                    } else {
                        in_quote = 0
                    }
                } else {
                    current_field = current_field char
                }
            } else {
                if (char == "\"") {
                    in_quote = 1
                } else if (char == ",") {
                    field_count++
                    fields[field_count] = current_field
                    current_field = ""
                } else if (char == "\t") {
                    # Tab in TSV context: escape it
                    current_field = current_field "\\t"
                } else if (char == "\n") {
                    # Should not happen due to RS, but safety
                    current_field = current_field "\\n"
                } else {
                    current_field = current_field char
                }
            }
        }
        
        # End of record (comma or not)
        field_count++
        fields[field_count] = current_field
        current_field = ""
        
        output_record()
    }
    ' "$file"
}