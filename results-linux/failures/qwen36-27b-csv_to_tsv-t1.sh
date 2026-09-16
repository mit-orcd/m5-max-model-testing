csv_to_tsv() {
    local file="$1"
    if [[ -z "$file" ]]; then
        return 1
    fi

    # Read the entire file content
    local content
    content=$(<"$file")

    # Process character by character
    local i=0
    local len=${#content}
    local field=""
    local in_quotes=0
    local first_field=1
    local record=""
    local prev_char=""

    while (( i < len )); do
        local char="${content:$i:1}"
        local next_char="${content:$((i+1)):1}"

        if (( in_quotes )); then
            if [[ "$char" == '"' ]]; then
                if [[ "$next_char" == '"' ]]; then
                    # Escaped quote: "" -> literal "
                    field+='"'
                    (( i++ )) # Skip next quote
                else
                    # End of quoted field
                    in_quotes=0
                fi
            else
                # Inside quoted field: handle special characters
                if [[ "$char" == $'\n' ]]; then
                    field+="\\n"
                elif [[ "$char" == $'\t' ]]; then
                    field+="\\t"
                elif [[ "$char" == $'\r' ]]; then
                    # Skip CR inside quoted field
                    :
                else
                    field+="$char"
                fi
            fi
        else
            # Not in quotes
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\r' ]]; then
                # Skip CR (part of CRLF or stray CR)
                :
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                if (( ! first_field )); then
                    record+="$field"
                else
                    record="$field"
                fi
                # Output the record with a trailing newline
                printf '%s\n' "$record"
                record=""
                field=""
                first_field=1
            elif [[ "$char" == ',' ]]; then
                # Field separator
                if (( first_field )); then
                    record="$field"
                    first_field=0
                else
                    record+=$'\t'"$field"
                fi
                field=""
            else
                field+="$char"
            fi
        fi
        (( i++ ))
    done

    # Handle last record if no trailing newline in input
    if [[ -n "$record" ]] || [[ -n "$field" ]] || (( ! first_field )); then
        if (( first_field )); then
            record="$field"
        else
            record+="$field"
        fi
        printf '%s\n' "$record"
    fi
}