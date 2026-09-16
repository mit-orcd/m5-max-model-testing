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
                    # Skip next char in next iteration
                    skip_next = 1
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
                fields[++fields_count] = field
                field = ""
            } else if (c == "\r") {
                # Handle CRLF: if next is LF, skip it
                if (next_char == "\n") {
                    skip_next = 1
                }
                # CR is ignored, effectively stripping it
            } else if (c == "\n") {
                # End of record
                fields[++fields_count] = field
                output_record()
                fields_count = 0
                field = ""
            } else {
                field = field c
            }
        }
    }

    {
        # Prepend previous line if any (handling multi-line fields)
        # Actually, we need to handle the buffer across lines
        # Since RS is \n, we process line by line, but fields can span lines
        # We need to accumulate the current record state
        
        # If we are in a quote, this line is a continuation
        # If not, it starts a new record (unless we are in the middle of a field from previous line)
        
        # We need to handle the fact that RS="\n" splits on newlines, 
        # but a newline inside quotes is part of the field.
        # So we must manually parse the line character by character.
        
        # Reset state if not in quote (start of new record)
        if (!in_quote && fields_count == 0) {
            # Start of a new record
            fields_count = 0
            field = ""
        }
        
        # Process characters in the current line
        n = length($0)
        for (i = 1; i <= n; i++) {
            c = substr($0, i, 1)
            # Determine next char for lookahead
            if (i < n) {
                next_char = substr($0, i+1, 1)
            } else {
                next_char = ""
            }
            
            # Handle skip_next from previous iteration
            if (skip_next) {
                skip_next = 0
                continue
            }

            if (in_quote) {
                if (c == "\"") {
                    if (next_char == "\"") {
                        field = field "\""
                        skip_next = 1
                    } else {
                        in_quote = 0
                    }
                } else {
                    # Inside quotes, preserve newlines and tabs as literal chars for now
                    # We will escape them at output time
                    field = field c
                }
            } else {
                if (c == "\"") {
                    in_quote = 1
                } else if (c == ",") {
                    fields[++fields_count] = field
                    field = ""
                } else if (c == "\r") {
                    # CR handling
                    if (next_char == "\n") {
                        skip_next = 1
                    }
                    # CR is consumed, do not add to field
                } else if (c == "\n") {
                    # End of record
                    fields[++fields_count] = field
                    output_record()
                    fields_count = 0
                    field = ""
                } else {
                    field = field c
                }
            }
        }
    }

    END {
        # Handle last record if it ended without newline
        if (fields_count > 0 || field != "") {
            if (in_quote) {
                # Unclosed quote, treat rest as field
                fields[++fields_count] = field
            } else {
                fields[++fields_count] = field
            }
            output_record()
        }
        # Ensure final newline
        if (!first_record) {
            printf "\n"
        }
    }
    ' "$file" | awk '
    BEGIN { FS = "\t"; OFS = "\t" }
    {
        for (i = 1; i <= NF; i++) {
            # Escape backslashes first to avoid double escaping
            gsub(/\\/, "\\\\", $i)
            # Escape tabs
            gsub(/\t/, "\\t", $i)
            # Escape newlines
            gsub(/\n/, "\\n", $i)
            printf "%s", $i
            if (i < NF) printf "\t"
        }
        printf "\n"
    }
    '
}