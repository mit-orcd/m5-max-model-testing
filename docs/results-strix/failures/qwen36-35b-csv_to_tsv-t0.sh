csv_to_tsv() {
    local file="${1:-/dev/stdin}"
    awk '
    BEGIN {
        FS = ""
        OFS = ""
        RS = "\n"
        ORS = "\n"
    }

    function process_record(record,    i, len, c, in_quotes, field, fields_count, result, prev_c) {
        len = length(record)
        in_quotes = 0
        field = ""
        fields_count = 0
        result = ""
        prev_c = ""

        for (i = 1; i <= len; i++) {
            c = substr(record, i, 1)

            if (in_quotes) {
                if (c == "\"") {
                    if (i < len && substr(record, i + 1, 1) == "\"") {
                        # Escaped quote ""
                        field = field "\""
                        i++
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
                    fields_count++
                    if (fields_count > 1) {
                        result = result "\t"
                    }
                    # Escape tab and newline in field
                    gsub(/\t/, "\\t", field)
                    gsub(/\n/, "\\n", field)
                    result = result field
                    field = ""
                } else {
                    field = field c
                }
            }
        }

        # Handle last field
        fields_count++
        if (fields_count > 1) {
            result = result "\t"
        }
        gsub(/\t/, "\\t", field)
        gsub(/\n/, "\\n", field)
        result = result field

        return result
    }

    {
        # Remove trailing CR if present
        gsub(/\r$/, "")

        # We need to handle records that might span multiple lines if quoted fields contain newlines
        # This is tricky with line-by-line processing. We need a stateful parser.
    }

    # We will use a different approach: read the entire file, then parse
    ' < "$file" 2>/dev/null

    # Since awk line-by-line is hard for quoted newlines, let's use a more robust method
    # We'll read the file into a variable and process it character by character or use a state machine
    # But bash loops are slow. Let's try a different awk approach that handles multi-line records.

    # Actually, let's implement a proper parser in awk that reads the whole file
    awk '
    BEGIN {
        # Read entire file into a string
        content = ""
    }
    {
        if (NR > 1) content = content "\n"
        content = content $0
    }
    END {
        # Remove trailing CRs from the content
        gsub(/\r$/, "", content)
        # Also remove any \r that might be in the middle (though RFC says CRLF or LF)
        gsub(/\r/, "", content)

        len = length(content)
        i = 1
        first_record = 1

        while (i <= len) {
            # Start of a new record
            field = ""
            in_quotes = 0
            record_done = 0
            output_line = ""
            field_count = 0

            while (i <= len && !record_done) {
                c = substr(content, i, 1)

                if (in_quotes) {
                    if (c == "\"") {
                        if (i < len && substr(content, i + 1, 1) == "\"") {
                            # Escaped quote
                            field = field "\""
                            i++
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
                        field_count++
                        if (field_count > 1) {
                            output_line = output_line "\t"
                        }
                        # Escape tab and newline in field for TSV output
                        gsub(/\t/, "\\t", field)
                        gsub(/\n/, "\\n", field)
                        output_line = output_line field
                        field = ""
                        i++
                    } else if (c == "\n") {
                        # End of record
                        record_done = 1
                        # Do not consume the newline, it will be handled by the outer loop
                        # Actually, we need to move past it
                        i++
                    } else {
                        field = field c
                        i++
                    }
                }
            }

            # Handle the last field of the record
            field_count++
            if (field_count > 1) {
                output_line = output_line "\t"
            }
            gsub(/\t/, "\\t", field)
            gsub(/\n/, "\\n", field)
            output_line = output_line field

            if (!first_record) {
                printf "\n"
            }
            printf "%s", output_line
            first_record = 0
        }
        printf "\n"
    }
    ' < "$file"
}