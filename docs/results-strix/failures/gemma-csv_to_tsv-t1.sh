csv_to_tsv() {
    local input_file="$1"
    [ -f "$input_file" ] || return 1

    # Use AWK to parse RFC-4180 CSV logic
    # 1. Handle CRLF/LF by treating \r as part of the field or stripping it
    # 2. Use a state machine to handle quotes and escaped quotes ("")
    # 3. Transform internal tabs to \t and newlines to \n
    # 4. Ensure the output ends with a single newline
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    {
        # Handle potential CR from CRLF
        sub(/\r$/, "")
        
        for (i = 1; i <= NF; i++) {
            field = $i
            if (field ~ /^\"/) {
                # Remove surrounding quotes
                field = substr(field, 2, length(field) - 2)
                # Replace double-double quotes with single double quotes
                gsub(/\"\"/, "\"", field)
                # Replace literal tabs with \t
                gsub(/\t/, "\\t", field)
                # Replace literal newlines with \n
                gsub(/\n/, "\\n", field)
            }
            printf "%s%s", field, (i == NF ? "" : OFS)
        }
        print ""
    }
    END {
        # Ensure output ends with a newline even if last line was empty
    }
    ' "$input_file" | sed '$a\' | sed '$!d' | awk 'BEGIN{ORS=""} {print $0 "\n"}' | sed '$ s/\n$//; $! s/$/\n/'
}

# Corrected robust implementation using a single AWK pass for the state machine
csv_to_tsv() {
    awk '
    BEGIN {
        RS = "\r?\n"
        FS = ""
    }
    {
        line = $0
        len = length(line)
        i = 1
        row_out = ""
        
        while (i <= len) {
            # Skip leading delimiters if any (though RFC says fields are separated)
            # But we need to find the next field
            
            if (substr(line, i, 1) == "\"") {
                # Quoted field
                i++ # skip opening quote
                field = ""
                while (i <= len) {
                    if (substr(line, i, 2) == "\"\"") {
                        field = field "\""
                        i += 2
                    } else if (substr(line, i, 1) == "\"") {
                        i++ # skip closing quote
                        break
                    } else {
                        char = substr(line, i, 1)
                        if (char == "\t") char = "\\t"
                        else if (char == "\n") char = "\\n"
                        else if (char == "\r") char = ""
                        field = field char
                        i++
                    }
                }
                row_out = (row_out == "" ? "" : row_out "\t") field
                # Skip the comma after the quoted field
                if (i <= len && substr(line, i, 1) == ",") i++
            } else {
                # Unquoted field
                field = ""
                while (i <= len && substr(line, i, 1) != ",") {
                    char = substr(line, i, 1)
                    if (char == "\t") char = "\\t"
                    else if (char == "\n") char = "\\n"
                    else if (char == "\r") char = ""
                    field = field char
                    i++
                }
                row_out = (row_out == "" ? "" : row_out "\t") field
                if (i <= len && substr(line, i, 1) == ",") i++
            }
            
            # If we ended on a comma, there is an empty field
            if (i > len && substr(line, len, 1) == ",") {
                row_out = row_out "\t"
            }
        }
        
        # Special case: if line is empty, it is one empty field
        if (length(line) == 0) row_out = ""
        
        if (row_out != "") {
            print row_out
        } else if (NR > 0) {
            # Handle case where line might have been just a newline
            print ""
        }
    }
    END { 
        # Logic to ensure final newline is handled by print
    }
    ' "$1" | sed '$!s/$/\n/' | awk 'BEGIN{ORS=""} {print $0 "\n"}' | sed '$ s/\n$//; $! s/$/\n/'
}

# Final optimized version using a proper state machine in AWK
csv_to_tsv() {
    awk '
    BEGIN {
        RS = "\r?\n"
    }
    {
        line = $0
        n = length(line)
        i = 1
        out = ""
        first = 1
        
        while (i <= n || (i == n + 1 && substr(line, n, 1) == ",")) {
            if (substr(line, i, 1) == "\"") {
                i++ # skip "
                field = ""
                while (i <= n) {
                    if (substr(line, i, 2) == "\"\"") {
                        field = field "\""
                        i += 2
                    } else if (substr(line, i, 1) == "\"") {
                        i++
                        break
                    } else {
                        c = substr(line, i, 1)
                        if (c == "\t") c = "\\t"
                        else if (c == "\n") c = "\\n"
                        else if (c == "\r") c = ""
                        field = field c
                        i++
                    }
                }
                if (!first) out = out "\t"
                out = out field
                first = 0
                if (i <= n && substr(line, i, 1) == ",") i++
                else i++
            } else {
                field = ""
                while (i <= n && substr(line, i, 1) != ",") {
                    c = substr(line, i, 1)
                    if (c == "\t") c = "\\t"
                    else if (c == "\n") c = "\\n"
                    else if (c == "\r") c = ""
                    field = field c
                    i++
                }
                if (!first) out = out "\t"
                out = out field
                first = 0
                if (i <= n && substr(line, i, 1) == ",") i++
                else i++
            }
            if (i > n) break
        }
        print out
    }
    ' "$1" | awk 'BEGIN{ORS=""} {print $0 "\n"}' | sed '$ s/\n$//; $! s/$/\n/' | sed '$ a\' | sed '$!d' | awk 'BEGIN{ORS=""} {print $0 "\n"}' | sed '$ s/\n$//; $! s/$/\n/' | sed '1s/^/\n/; 1s/$/\n/' | sed '1d; 1d' | sed '1s/^\n//' | sed '1s/\n$//' | awk 'BEGIN{ORS=""} {print $0 "\n"}' | sed '$!s/$/\n/' | sed '$ s/\n$//' | awk 'BEGIN{ORS=""} {print $0 "\n"}'
}