csv_to_tsv() {
    local file="$1"
    local field=""
    local in_quotes=0
    local c prev_c=""
    local line=""
    local output_line=""
    local first_field=1
    local has_data=0

    # Read the file character by character to handle embedded newlines and CR/LF
    while IFS= read -r -d '' -n 1 c || [[ -n "$c" ]]; do
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$c" == '"' ]]; then
                if [[ "$prev_c" == '"' ]]; then
                    # Escaped quote: add a single quote to the field
                    field+='"'
                    prev_c='"'
                    continue
                else
                    # End of quoted field
                    in_quotes=0
                    prev_c='"'
                    continue
                fi
            else
                if [[ "$c" == $'\t' ]]; then
                    field+="\\t"
                elif [[ "$c" == $'\n' ]]; then
                    field+="\\n"
                elif [[ "$c" == $'\r' ]]; then
                    # CR inside quoted field: treat as part of line ending if followed by LF, 
                    # but since we read char by char and handle LF explicitly, 
                    # RFC 4180 says CRLF inside quotes is a newline. 
                    # We've already handled LF separately. 
                    # If we see CR here, it's likely a stray CR or part of CRLF.
                    # Since we consume LF separately, a lone CR in a quoted field 
                    # should probably be treated as a newline or ignored? 
                    # RFC 4180: CRLF is the line break. Inside quotes, CRLF is a newline.
                    # Our read loop sees CR then LF. 
                    # We handle LF as \\n. So if we see CR, we should wait for LF?
                    # Actually, the requirement says "line break inside a quoted field is written as \\n".
                    # And "CR must not survive".
                    # So if we see CR, we ignore it (assuming it's part of CRLF).
                    # If it's a lone CR, it's ambiguous, but ignoring is safe for "CR not survive".
                    prev_c="$c"
                    continue
                else
                    field+="$c"
                fi
                prev_c="$c"
                continue
            fi
        else
            # Not in quotes
            if [[ "$c" == '"' ]]; then
                in_quotes=1
                prev_c='"'
                continue
            elif [[ "$c" == $'\r' ]]; then
                # CR: ignore, assuming CRLF. If lone CR, it's an edge case.
                # If it ends the line, the next char will be LF or EOF.
                prev_c="$c"
                continue
            elif [[ "$c" == $'\n' ]]; then
                # End of record
                # Output the current field
                if [[ $first_field -eq 1 ]]; then
                    output_line="$field"
                    first_field=0
                else
                    output_line+=$'\t'"$field"
                fi
                echo "$output_line"
                # Reset for next record
                field=""
                output_line=""
                first_field=1
                prev_c="$c"
                has_data=1
                continue
            elif [[ "$c" == ',' ]]; then
                # Field separator
                if [[ $first_field -eq 1 ]]; then
                    output_line="$field"
                    first_field=0
                else
                    output_line+=$'\t'"$field"
                fi
                field=""
                prev_c="$c"
                continue
            else
                field+="$c"
                prev_c="$c"
                continue
            fi
        fi
    done < "$file"

    # Handle the last record if it didn't end with a newline
    if [[ $has_data -eq 1 ]] || [[ -n "$field" ]] || [[ -n "$output_line" ]]; then
        if [[ $first_field -eq 1 ]]; then
            output_line="$field"
        else
            output_line+=$'\t'"$field"
        fi
        echo "$output_line"
    fi
}