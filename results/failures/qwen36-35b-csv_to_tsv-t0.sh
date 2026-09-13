csv_to_tsv() {
    local file="$1"
    if [[ -z "$file" ]]; then
        echo "Usage: csv_to_tsv <file>" >&2
        return 1
    fi

    if [[ ! -f "$file" ]]; then
        echo "Error: File '$file' not found." >&2
        return 1
    fi

    awk '
    BEGIN {
        FS = ""
        OFS = ""
        in_quotes = 0
        field = ""
        record = ""
        first_field = 1
    }

    function process_char(c) {
        if (in_quotes) {
            if (c == "\"") {
                # Check for escaped quote ""
                if (next_char == "\"") {
                    field = field "\""
                    next_char = ""
                } else {
                    # End of quoted field
                    in_quotes = 0
                }
            } else if (c == "\n") {
                # Newline inside quoted field -> \n
                field = field "\\n"
            } else if (c == "\t") {
                # Tab inside quoted field -> \t
                field = field "\\t"
            } else {
                field = field c
            }
        } else {
            if (c == "\"") {
                in_quotes = 1
            } else if (c == ",") {
                # End of field
                if (!first_field) {
                    record = record "\t"
                }
                record = record field
                field = ""
                first_field = 0
            } else if (c == "\n") {
                # End of record
                if (!first_field) {
                    record = record "\t"
                }
                record = record field
                print record
                record = ""
                field = ""
                first_field = 1
            } else if (c == "\r") {
                # Skip CR (CRLF handling)
            } else {
                field = field c
            }
        }
    }

    {
        line = $0
        # Remove trailing CR if present (CRLF)
        sub(/\r$/, "", line)
        
        n = length(line)
        for (i = 1; i <= n; i++) {
            c = substr(line, i, 1)
            if (i < n) {
                next_char = substr(line, i + 1, 1)
            } else {
                next_char = ""
            }
            process_char(c)
        }
    }

    END {
        # Handle case where file does not end with newline
        if (field != "" || record != "") {
            if (!first_field) {
                record = record "\t"
            }
            record = record field
            print record
        }
    }
    ' "$file"
}