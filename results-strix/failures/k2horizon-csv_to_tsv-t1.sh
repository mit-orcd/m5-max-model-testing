csv_to_tsv() {
    local file="$1"
    local line
    local in_field=0
    local in_quotes=0
    local field=""
    local out_line=""
    local i c prev

    while IFS= read -r line || [ -n "$line" ]; do
        # Remove trailing CR (handles CRLF); strip it before processing
        line="${line%$'\r'}"

        # If we are already inside a quoted field, this line is a continuation
        if [ "$in_quotes" -eq 1 ]; then
            field+=$'\n'"$line"
        else
            field=""
            # Process the line character by character
            for ((i = 0; i < ${#line}; i++)); do
                c="${line:i:1}"
                if [ "$in_quotes" -eq 1 ]; then
                    if [ "$c" = '"' ]; then
                        # Check for escaped double quote
                        if [ $((i + 1)) -lt ${#line} ] && [ "${line:i+1:1}" = '"' ]; then
                            field+='"'
                            i=$((i + 1))
                        else
                            in_quotes=0
                        fi
                    else
                        field+="$c"
                    fi
                else
                    case "$c" in
                        '"')
                            in_quotes=1
                            ;;
                        ',')
                            # End of field
                            out_line+="$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                            out_line+=$'\t'
                            field=""
                            ;;
                        $'\n')
                            # End of record (shouldn't happen since read strips it, but handle anyway)
                            out_line+="$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                            printf '%s\n' "${out_line%$'\t'}"
                            out_line=""
                            field=""
                            ;;
                        *)
                            field+="$c"
                            ;;
                    esac
                fi
            done
        fi

        # If we are at end of line and not in quotes, end the field
        if [ "$in_quotes" -eq 0 ]; then
            out_line+="$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
            printf '%s\n' "${out_line%$'\t'}"
            out_line=""
            field=""
        fi
    done < "$file"

    # Handle any remaining field/record (e.g., file doesn't end with newline)
    if [ -n "$out_line" ] || [ -n "$field" ]; then
        printf '%s\n' "${out_line%$'\t'}"
    fi
}