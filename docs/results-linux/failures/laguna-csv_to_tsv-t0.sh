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
            if [[ -n "$(head -c1 <<<"$char")" ]]; then
                # Read next char to check for LF
                local next_char
                read -r -n1 next_char
                if [[ "$next_char" == $'\n' ]]; then
                    char=$'\n'
                else
                    # Not CRLF, process CR as normal
                    char=$'\r'
                    # Put back the next_char
                    read -r -n1 next_char
                fi
            fi
        fi

        # Handle end of line (LF or end of file)
        if [[ "$char" == $'\n' ]] || [[ -z "$char" ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # End of line inside quoted field, add as is
                field+="$char"
            else
                # End of record
                line+="$field"
                field=""
                # Output the line
                echo "$line"
                line=""
            fi
            prev_char=""
            crlf_seen=0
            continue
        fi

        # Handle carriage return (not part of CRLF)
        if [[ "$char" == $'\r' ]]; then
            if [[ $in_quotes -eq 0 ]]; then
                # Not in quotes, CR is line ending
                line+="$field"
                field=""
                echo "$line"
                line=""
                prev_char=""
                continue
            else
                # In quotes, keep the CR
                field+="$char"
                prev_char="$char"
                continue
            fi
        fi

        # Handle quotes
        if [[ "$char" == '"' ]]; then
            if [[ $in_quotes -eq 1 ]] && [[ "$prev_char" == '"' ]]; then
                # Escaped quote
                field+='"'
                prev_char=""
                continue
            elif [[ $in_quotes -eq 1 ]]; then
                # End of quoted field
                in_quotes=0
                prev_char=""
                continue
            else
                # Start of quoted field
                in_quotes=1
                prev_char=""
                continue
            fi
        fi

        # Handle tab
        if [[ "$char" == $'\t' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                field+='\t'
            else
                # End of field
                line+="$field"$'\t'
                field=""
            fi
            prev_char=""
            continue
        fi

        # Regular character
        field+="$char"
        prev_char="$char"
    done < "$file"

    # Handle last record if no trailing newline
    if [[ -n "$field" ]] || [[ -n "$line" ]]; then
        line+="$field"
        echo "$line"
    fi
}