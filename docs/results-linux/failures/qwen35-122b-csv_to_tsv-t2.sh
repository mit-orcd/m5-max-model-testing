csv_to_tsv() {
    local file="$1"
    local in_quote=0
    local field=""
    local line=""
    local char
    local prev_char=""

    while IFS= read -r -d '' -n 1 char || [[ -n "$char" ]]; do
        # Handle CRLF -> LF
        if [[ "$char" == $'\r' ]]; then
            prev_char="$char"
            continue
        fi
        if [[ "$prev_char" == $'\r' ]]; then
            prev_char=""
        fi

        if [[ $in_quote -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # Escaped quote ""
                    field+=$'\t' # Placeholder, will be handled later
                    field="${field%$'\t'}" # Remove placeholder
                    field+='"'
                else
                    # End of quote
                    in_quote=0
                fi
            else
                field+="$char"
            fi
        else
            if [[ "$char" == '"' ]]; then
                # Start of quote (assuming field starts with quote if quoted)
                in_quote=1
            elif [[ "$char" == ',' ]]; then
                # Field separator
                # Escape special chars in field for TSV
                field="${field//\\/\\\\}"
                field="${field//$'\t'/\\t}"
                field="${field//$'\n'/\\n}"
                line+="$field"$'\t'
                field=""
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                field="${field//\\/\\\\}"
                field="${field//$'\t'/\\t}"
                field="${field//$'\n'/\\n}"
                line+="$field"
                printf '%s\n' "$line"
                line=""
                field=""
            else
                field+="$char"
            fi
        fi
        prev_char="$char"
    done < "$file"

    # Handle last record if no trailing newline
    if [[ -n "$field" || -n "$line" ]]; then
        field="${field//\\/\\\\}"
        field="${field//$'\t'/\\t}"
        field="${field//$'\n'/\\n}"
        line+="$field"
        printf '%s\n' "$line"
    fi
}