csv_to_tsv() {
    local file="$1"
    local line
    local record=""
    local field=""
    local in_quotes=0
    local prev_char=""
    local char
    local i
    local len
    local content

    # Read the entire file, stripping CR characters
    content=$(cat "$file" | tr -d '\r')
    # Add a trailing newline if not present to ensure last record is processed
    if [[ -n "$content" && "${content: -1}" != $'\n' ]]; then
        content+=$'\n'
    fi

    # Process character by character
    len=${#content}
    for (( i=0; i<len; i++ )); do
        char="${content:$i:1}"

        if (( in_quotes )); then
            if [[ "$char" == '"' ]]; then
                # Check if next char is also a quote (escaped quote)
                if (( i+1 < len )) && [[ "${content:$((i+1)):1}" == '"' ]]; then
                    field+='"'
                    (( i++ ))
                else
                    # End of quoted field
                    in_quotes=0
                fi
            else
                if [[ "$char" == $'\n' ]]; then
                    field+='\n'
                elif [[ "$char" == $'\t' ]]; then
                    field+='\t'
                else
                    field+="$char"
                fi
            fi
        else
            if [[ "$char" == '"' ]]; then
                # Start of quoted field
                in_quotes=1
            elif [[ "$char" == ',' ]]; then
                # End of field
                record+="$field"$'\t'
                field=""
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                # Remove trailing tab
                record="${record%$'\t'}"
                printf '%s\n' "$record"
                record=""
                field=""
            else
                field+="$char"
            fi
        fi
    done
}