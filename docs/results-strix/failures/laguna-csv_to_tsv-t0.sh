csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local line=""
    local char
    local prev_char=""
    local crlf_seen=0

    # Read file character by character
    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        # Handle CRLF line endings
        if [[ "$char" == $'\r' ]]; then
            # Peek next character
            if IFS= read -r -n1 next_char; then
                if [[ "$next_char" == $'\n' ]]; then
                    char=$'\n'
                    crlf_seen=1
                else
                    # Not CRLF, process \r as normal char
                    if [[ $in_quotes -eq 1 ]]; then
                        field+="$char"
                    fi
                    char="$next_char"
                fi
            else
                # End of file
                if [[ $in_quotes -eq 1 ]]; then
                    field+="$char"
                fi
            fi
        fi

        # Handle end of line
        if [[ "$char" == $'\n' && $in_quotes -eq 0 ]]; then
            # Output the line
            if [[ -n "$line" ]]; then
                echo "$line"
            else
                echo -e "$field"
            fi
            line=""
            field=""
            prev_char=""
            continue
        fi

        # Inside quoted field
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # Escaped quote
                    field+='"'
                    prev_char=""
                else
                    # Toggle quotes
                    in_quotes=0
                fi
            else
                field+="$char"
            fi
        else
            # Outside quoted field
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\t' ]]; then
                # Tab outside quotes - error in CSV, but treat as field separator
                if [[ -n "$line" ]]; then
                    line+=$'\t'
                else
                    line="$field"
                fi
                field=""
            elif [[ "$char" == $'\n' ]]; then
                # Newline outside quotes - end of record
                if [[ -n "$line" ]]; then
                    line+=$'\t'
                else
                    line="$field"
                fi
                echo "$line"
                line=""
                field=""
            elif [[ "$char" == ',' ]]; then
                # Comma outside quotes - field separator
                if [[ -n "$line" ]]; then
                    line+=$'\t'
                else
                    line="$field"
                fi
                field=""
            else
                field+="$char"
            fi
        fi

        prev_char="$char"
        crlf_seen=0
    done < "$file"

    # Handle last record without trailing newline
    if [[ -n "$field" || -n "$line" ]]; then
        if [[ -n "$line" ]]; then
            line+=$'\t'
        fi
        line+="$field"
        echo "$line"
    fi
}