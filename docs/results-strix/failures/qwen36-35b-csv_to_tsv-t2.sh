csv_to_tsv() {
    local file="$1"
    if [[ -z "$file" ]]; then
        return 1
    fi

    awk '
    BEGIN {
        FS = "\n"
        ORS = ""
        record = ""
        field = ""
        in_quotes = 0
        first_field = 1
        first_record = 1
    }

    function emit_field() {
        if (!first_field) {
            printf "\t"
        }
        printf "%s", field
        first_field = 0
    }

    function emit_record() {
        if (!first_record) {
            printf "\n"
        }
        printf "%s", record
        first_record = 0
        record = ""
        field = ""
        first_field = 1
    }

    {
        line = $0
        # Remove trailing CR if present
        if (substr(line, length(line), 1) == "\r") {
            line = substr(line, 1, length(line) - 1)
        }

        n = length(line)
        i = 1
        while (i <= n) {
            c = substr(line, i, 1)

            if (in_quotes) {
                if (c == "\"") {
                    # Check for escaped quote ""
                    if (i < n && substr(line, i + 1, 1) == "\"") {
                        field = field "\""
                        i += 2
                    } else {
                        # End of quoted field
                        in_quotes = 0
                        i++
                    }
                } else {
                    if (c == "\n") {
                        field = field "\\n"
                    } else if (c == "\t") {
                        field = field "\\t"
                    } else {
                        field = field c
                    }
                    i++
                }
            } else {
                if (c == "\"") {
                    in_quotes = 1
                    i++
                } else if (c == ",") {
                    emit_field()
                    field = ""
                    i++
                } else if (c == "\n") {
                    # Should not happen in well-formed CSV with proper line handling,
                    # but if we are not in quotes and hit newline, end of record
                    emit_field()
                    emit_record()
                } else {
                    field = field c
                    i++
                }
            }
        }

        if (in_quotes && i > n) {
            # Quoted field continues to next line
            field = field "\\n"
        }
    }

    END {
        if (!first_field || !first_record) {
            emit_field()
            emit_record()
        }
        if (!first_record) {
            printf "\n"
        }
    }
    ' "$file"
}