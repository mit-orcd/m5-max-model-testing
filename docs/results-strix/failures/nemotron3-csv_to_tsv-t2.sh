csv_to_tsv() {
    local file="$1"
    local line field in_quotes=0 quoted field_buf output_line i c prev
    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%%$'\r'}"  # Strip CR if present (CRLF -> LF)
        field_buf=""
        output_line=""
        in_quotes=0
        i=0
        while (( i < ${#line} )); do
            c="${line:i:1}"
            if (( !in_quotes )); then
                if [[ "$c" == '"' ]]; then
                    in_quotes=1
                elif [[ "$c" == ',' ]]; then
                    output_line+="${field_buf}$'\t'"
                    field_buf=""
                else
                    field_buf+="$c"
                fi
            else
                if [[ "$c" == '"' ]]; then
                    (( i+1 < ${#line} )) && [[ "${line:i+1:1}" == '"' ]] && {
                        field_buf+='"'
                        ((i++))  # Skip next quote
                    } || {
                        in_quotes=0
                    }
                else
                    field_buf+="$c"
                fi
            fi
            ((i++))
        done
        # After loop, if still in quotes, treat as unterminated (invalid per RFC-4180, but handle gracefully)
        if (( in_quotes )); then
            # Unterminated quote: treat rest as literal (though spec says this shouldn't happen)
            :
        fi
        output_line+="${field_buf}"
        # Escape tabs and newlines in field content for TSV
        output_line="${output_line//$'\t'/\\t}"
        output_line="${output_line//$'\n'/\\n}"
        printf '%s\n' "$output_line"
    done < "$file"
}