csv_to_tsv() {
    local file="$1"
    awk -v RS='' '
    BEGIN {
        FS = ""
        OFS = ""
    }
    {
        # Remove CR characters to handle CRLF
        gsub(/\r/, "")
        n = length($0)
        i = 1
        first_field = 1
        while (i <= n || !first_field) {
            if (!first_field) {
                printf "\t"
            }
            first_field = 0

            if (i > n) {
                # Empty field at end (trailing comma)
                i++
                continue
            }

            c = substr($0, i, 1)

            if (c == "\"") {
                # Quoted field
                i++
                field = ""
                while (i <= n) {
                    c = substr($0, i, 1)
                    if (c == "\"") {
                        # Check next char
                        if (i + 1 <= n && substr($0, i + 1, 1) == "\"") {
                            field = field "\""
                            i += 2
                        } else {
                            # End of quoted field
                            i++ # skip closing quote
                            break
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
                }
                printf "%s", field
            } else {
                # Unquoted field
                field = ""
                while (i <= n) {
                    c = substr($0, i, 1)
                    if (c == "," || c == "\n" || c == "\r") {
                        break
                    }
                    field = field c
                    i++
                }
                printf "%s", field
            }

            # After field, expect comma, newline, or end
            if (i <= n) {
                c = substr($0, i, 1)
                if (c == ",") {
                    i++
                } else if (c == "\n") {
                    # End of record, handled by loop condition
                    i++
                    break
                } else if (c == "\r") {
                    # Should have been stripped, but just in case
                    i++
                    break
                } else {
                    # Unexpected character, treat as end of field
                    break
                }
            }
        }
        printf "\n"
    }
    ' "$file"
}