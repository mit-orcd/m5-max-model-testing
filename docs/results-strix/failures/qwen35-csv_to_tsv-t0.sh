csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = "\n"
        FS = ""
        ORS = ""
        in_quote = 0
        field = ""
        fields_count = 0
        first_record = 1
    }

    function output_record() {
        if (fields_count > 0) {
            if (!first_record) printf "\n"
            first_record = 0
            for (i = 1; i <= fields_count; i++) {
                if (i > 1) printf "\t"
                printf "%s", fields[i]
            }
        }
    }

    function process_char(c) {
        if (in_quote) {
            if (c == "\"") {
                if (next_char == "\"") {
                    field = field "\""
                    next_char = ""
                } else {
                    in_quote = 0
                }
            } else {
                field = field c
            }
        } else {
            if (c == "\"") {
                in_quote = 1
            } else if (c == ",") {
                fields[fields_count + 1] = field
                fields_count++
                field = ""
            } else if (c == "\n") {
                fields[fields_count + 1] = field
                fields_count++
                output_record()
                fields_count = 0
                field = ""
            } else if (c == "\r") {
                # Ignore CR, handled by RS or next char
            } else {
                field = field c
            }
        }
    }

    {
        # Remove trailing CR if present (handles CRLF)
        gsub(/\r$/, "")
        
        # Prepend newline if not first line of file (to handle records split across lines)
        # But since RS is \n, we process line by line.
        # We need to handle the case where a record spans multiple lines.
        
        # We iterate character by character
        n = length($0)
        for (i = 1; i <= n; i++) {
            c = substr($0, i, 1)
            next_char = ""
            if (i < n) {
                next_char = substr($0, i+1, 1)
            }
            process_char(c)
        }
        
        # If we are still in a quote at end of line, we continue to next line
        # If not in quote, we have a complete record (or partial if empty)
        if (!in_quote) {
            # If we have accumulated fields, output them
            if (fields_count > 0) {
                output_record()
                fields_count = 0
                field = ""
            } else if (length($0) == 0 && NR > 1) {
                # Empty line between records? RFC 4180 says empty lines are not allowed 
                # but if present, treat as empty record? 
                # Actually, if not in quote and line is empty, it might be end of record.
                # But if we just finished a record, fields_count is 0.
                # If we are at start of file and empty line, ignore.
            }
        }
    }

    END {
        if (fields_count > 0) {
            output_record()
            printf "\n"
        } else if (!first_record) {
            # If last record ended without newline but was processed
            # The output_record adds newline, but we need to ensure final newline
            # If the file ended with a newline, output_record was called.
            # If file ended without newline, we need to output.
            # Wait, if in_quote is false at end, we output.
            # If in_quote is true, it is an error, but we assume valid input.
            # If we are here and first_record is false, we already printed newline in output_record?
            # No, output_record prints newline BEFORE the record.
            # We need to ensure the file ends with a newline.
            # If the last record was output, it ended with a newline?
            # Let's adjust output_record to not print newline at end of file, 
            # but print newline between records.
            # Actually, simpler: just print a newline at the very end if we printed anything.
            if (!first_record) {
                # If we printed records, we need to ensure the last one has a newline.
                # My output_record prints newline BEFORE the record.
                # So the last record printed does not have a trailing newline from that function.
                # We need to add it here.
                printf "\n"
            }
        }
    }
    ' "$file"
}