csv_to_tsv() {
    local file="$1"
    # Check if file argument is provided
    if [[ -z "$file" ]]; then
        echo "Usage: csv_to_tsv <file>" >&2
        return 1
    fi

    # Check if file exists
    if [[ ! -f "$file" ]]; then
        echo "Error: File '$file' not found" >&2
        return 1
    fi

    awk '
    BEGIN {
        FS = ""
        OFS = ""
        in_record = 0
        in_field = 0
        in_quotes = 0
        field = ""
        record = ""
        first_field = 1
    }

    {
        # Remove CR if present (handle CRLF)
        gsub(/\r$/, "")
        line = $0
        len = length(line)
        i = 1

        while (i <= len) {
            c = substr(line, i, 1)

            if (!in_field) {
                # Starting a new field
                in_field = 1
                field = ""
                if (c == "\"") {
                    in_quotes = 1
                    i++
                    continue
                }
            }

            if (in_quotes) {
                if (c == "\"") {
                    # Check if next char is also a quote
                    if (i < len && substr(line, i+1, 1) == "\"") {
                        # Escaped quote: "" -> "
                        field = field "\""
                        i += 2
                        continue
                    } else {
                        # End of quoted field
                        in_quotes = 0
                        i++
                        continue
                    }
                } else if (c == "\n") {
                    # Newline inside quoted field -> \n
                    field = field "\\n"
                    i++
                    continue
                } else if (c == "\t") {
                    # Tab inside quoted field -> \t
                    field = field "\\t"
                    i++
                    continue
                } else {
                    field = field c
                    i++
                    continue
                }
            } else {
                # Not in quotes
                if (c == ",") {
                    # End of field
                    if (!first_field) {
                        record = record "\t" field
                    } else {
                        record = field
                        first_field = 0
                    }
                    field = ""
                    in_field = 0
                    i++
                    continue
                } else if (c == "\n") {
                    # End of record
                    if (!first_field) {
                        record = record "\t" field
                    } else {
                        record = field
                        first_field = 0
                    }
                    # Output the record
                    printf "%s\n", record
                    record = ""
                    first_field = 1
                    in_field = 0
                    field = ""
                    i++
                    continue
                } else {
                    field = field c
                    i++
                    continue
                }
            }
        }

        # If we ended the line while still in a quoted field, the newline is part of the field
        if (in_quotes) {
            # The newline was already consumed in the loop if it was inside quotes
            # But actually, if we hit \n inside quotes, we handled it above
            # If we hit EOF while in_quotes, we need to handle it after the loop
        }
    }

    END {
        # Handle any remaining data
        if (in_field || in_quotes) {
            # If we were in quotes, the last field might have trailing quote issues
            # But our logic should have handled it
            if (!first_field) {
                record = record "\t" field
            } else {
                record = field
                first_field = 0
            }
            if (record != "") {
                printf "%s\n", record
            }
        }
    }
    ' "$file"
}