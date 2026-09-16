csv_to_tsv() {
    local file="$1"
    [[ -z "$file" ]] && return 1

    awk '
    BEGIN {
        FS = ""
        OFS = ""
        RS = "" # Read entire file as one record to handle multiline fields
    }
    {
        # Normalize CRLF to LF
        gsub(/\r\n/, "\n")
        gsub(/\r/, "\n")

        n = length($0)
        out = ""
        i = 1
        while (i <= n) {
            c = substr($0, i, 1)
            if (c == "\"") {
                # Quoted field
                i++
                field = ""
                while (i <= n) {
                    c = substr($0, i, 1)
                    if (c == "\"") {
                        if (substr($0, i+1, 1) == "\"") {
                            # Escaped quote
                            field = field "\""
                            i += 2
                        } else {
                            # End of quoted field
                            i++
                            break
                        }
                    } else if (c == "\n") {
                        # Line break inside quoted field -> \n
                        field = field "\\n"
                        i++
                    } else if (c == "\t") {
                        # Tab inside quoted field -> \t
                        field = field "\\t"
                        i++
                    } else {
                        field = field c
                        i++
                    }
                }
                out = out (out == "" ? "" : "\t") field
                # Skip comma if present after closing quote
                if (i <= n && substr($0, i, 1) == ",") {
                    i++
                }
            } else if (c == ",") {
                # Empty field or separator
                if (out != "") out = out "\t"
                i++
            } else {
                # Unquoted field
                field = ""
                while (i <= n && substr($0, i, 1) != "," && substr($0, i, 1) != "\n") {
                    c = substr($0, i, 1)
                    if (c == "\n") {
                        field = field "\\n"
                        i++
                    } else if (c == "\t") {
                        field = field "\\t"
                        i++
                    } else {
                        field = field c
                        i++
                    }
                }
                out = out (out == "" ? "" : "\t") field
            }
        }
        # Handle trailing newline logic: ensure exactly one newline at the end of output
        # Since RS="" reads the whole file, we just print the constructed line
        print out
    }
    ' "$file"
}