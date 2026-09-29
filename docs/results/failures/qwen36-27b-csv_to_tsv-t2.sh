csv_to_tsv() {
    local file="$1"
    if [[ -z "$file" ]]; then
        echo "Usage: csv_to_tsv <file>" >&2
        return 1
    fi

    # Read entire file into a variable, preserving content but handling line endings
    local content
    content=$(cat "$file")

    # Process the CSV content using awk
    echo "$content" | awk '
    BEGIN {
        RS = ""  # Not really used, we handle parsing manually
        state = "field"
        field = ""
        record = ""
        field_count = 0
        in_quotes = 0
    }

    {
        line = $0
        # Remove trailing CR if present (handle CRLF)
        sub(/\r$/, "", line)
        
        # If this is not the last line, there was a newline after this line
        # We need to know if there is a next line to determine if this line ends with a record boundary
        # However, awk processes line by line. We need to handle multi-line quoted fields.
        
        n = length(line)
        for (i = 1; i <= n; i++) {
            c = substr(line, i, 1)
            
            if (in_quotes) {
                if (c == "\"") {
                    # Check if next char is also a quote
                    if (i < n && substr(line, i+1, 1) == "\"") {
                        field = field "\""
                        i++ # Skip next quote
                    } else {
                        # End of quoted field
                        in_quotes = 0
                    }
                } else {
                    field = field c
                }
            } else {
                if (c == "\"") {
                    in_quotes = 1
                } else if (c == ",") {
                    # End of field
                    record = record field "\t"
                    field = ""
                    field_count++
                } else {
                    field = field c
                }
            }
        }
        
        # If we are still in quotes, the field continues to the next line
        if (in_quotes) {
            # Add a newline to the field (but we will escape it later)
            field = field "\n"
        } else {
            # End of field and end of record (newline in CSV means end of field and record if not in quotes)
            record = record field "\t"
            field = ""
            field_count++
            
            # Process the record
            # Remove trailing tab
            sub(/\t$/, "", record)
            
            # Escape tabs and newlines in each field
            # Split record by tab
            n_fields = split(record, fields_arr, "\t")
            escaped_record = ""
            for (j = 1; j <= n_fields; j++) {
                f = fields_arr[j]
                # Escape backslashes first? No, the spec says tab becomes \t and newline becomes \n
                # But what if the field contains a literal backslash? 
                # The spec says: "a line break inside a quoted field is written as the two characters backslash n"
                # and "a tab inside a field as the two characters backslash t"
                # It does not mention escaping existing backslashes. So we just replace literal tabs and newlines.
                
                # Replace literal tabs with \t
                gsub(/\t/, "\\t", f)
                # Replace literal newlines with \n
                gsub(/\n/, "\\n", f)
                
                if (j > 1) {
                    escaped_record = escaped_record "\t"
                }
                escaped_record = escaped_record f
            }
            
            print escaped_record
            record = ""
        }
    }

    END {
        # Handle the last record if it had no trailing newline
        # If we ended with a field that was not terminated by a comma and the file ended,
        # we need to output it.
        if (record != "" || field != "") {
            record = record field
            sub(/\t$/, "", record)
            if (record != "") {
                n_fields = split(record, fields_arr, "\t")
                escaped_record = ""
                for (j = 1; j <= n_fields; j++) {
                    f = fields_arr[j]
                    gsub(/\t/, "\\t", f)
                    gsub(/\n/, "\\n", f)
                    if (j > 1) {
                        escaped_record = escaped_record "\t"
                    }
                    escaped_record = escaped_record f
                }
                print escaped_record
            }
        }
    }
    '
}