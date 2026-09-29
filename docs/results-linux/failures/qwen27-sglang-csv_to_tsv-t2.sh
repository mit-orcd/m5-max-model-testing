csv_to_tsv() {
    local file="$1"
    local -a lines=()
    local content
    content=$(cat -- "$file"; printf x)
    content=${content%x}

    # Split into records, handling quoted fields that may contain newlines
    local record=""
    local in_quotes=0
    local i=0
    local len=${#content}
    local -a records=()

    while (( i < len )); do
        local ch="${content:$i:1}"
        if (( in_quotes )); then
            if [[ "$ch" == '"' ]]; then
                # Check for escaped quote ""
                if (( i + 1 < len )) && [[ "${content:$((i+1)):1}" == '"' ]]; then
                    record+='"'
                    (( i += 2 ))
                else
                    in_quotes=0
                    (( i++ ))
                fi
            else
                record+="$ch"
                (( i++ ))
            fi
        else
            if [[ "$ch" == '"' ]]; then
                in_quotes=1
                (( i++ ))
            elif [[ "$ch" == $'\n' ]]; then
                # End of record
                # Strip trailing CR if present (CRLF)
                if [[ "$record" == *$'\r' ]]; then
                    record="${record%$'\r'}"
                fi
                records+=("$record")
                record=""
                (( i++ ))
            elif [[ "$ch" == $'\r' ]]; then
                # Lone CR, treat as end of line if followed by LF or end
                if (( i + 1 < len )) && [[ "${content:$((i+1)):1}" == $'\n' ]]; then
                    # CRLF - skip both, end record
                    if [[ "$record" == *$'\r' ]]; then
                        record="${record%$'\r'}"
                    fi
                    records+=("$record")
                    record=""
                    (( i += 2 ))
                else
                    # Lone CR, treat as line ending
                    if [[ "$record" == *$'\r' ]]; then
                        record="${record%$'\r'}"
                    fi
                    records+=("$record")
                    record=""
                    (( i++ ))
                fi
            else
                record+="$ch"
                (( i++ ))
            fi
        fi
    done

    # Handle last record if no trailing newline
    if [[ -n "$record" ]]; then
        if [[ "$record" == *$'\r' ]]; then
            record="${record%$'\r'}"
        fi
        records+=("$record")
    fi

    # Now process each record: split by commas respecting quotes
    # We need to re-parse each record to split into fields
    # Actually, we should have tracked field boundaries during the first pass.
    # Let me redo this more carefully.

    # Reset and do it properly in one pass
    records=()
    record=""
    in_quotes=0
    i=0
    len=${#content}
    local -a fields=()
    local field=""

    while (( i < len )); do
        local ch="${content:$i:1}"
        if (( in_quotes )); then
            if [[ "$ch" == '"' ]]; then
                if (( i + 1 < len )) && [[ "${content:$((i+1)):1}" == '"' ]]; then
                    field+='"'
                    (( i += 2 ))
                else
                    in_quotes=0
                    (( i++ ))
                fi
            else
                field+="$ch"
                (( i++ ))
            fi
        else
            if [[ "$ch" == '"' ]]; then
                in_quotes=1
                (( i++ ))
            elif [[ "$ch" == ',' ]]; then
                fields+=("$field")
                field=""
                (( i++ ))
            elif [[ "$ch" == $'\n' ]]; then
                fields+=("$field")
                field=""
                # Strip trailing CR from last field if CRLF
                local last_idx=$(( ${#fields[@]} - 1 ))
                if [[ "${fields[$last_idx]}" == *$'\r' ]]; then
                    fields[$last_idx]="${fields[$last_idx]%$'\r'}"
                fi
                records+=("${fields[*]}")
                fields=()
                (( i++ ))
            elif [[ "$ch" == $'\r' ]]; then
                if (( i + 1 < len )) && [[ "${content:$((i+1)):1}" == $'\n' ]]; then
                    fields+=("$field")
                    field=""
                    local last_idx=$(( ${#fields[@]} - 1 ))
                    if [[ "${fields[$last_idx]}" == *$'\r' ]]; then
                        fields[$last_idx]="${fields[$last_idx]%$'\r'}"
                    fi
                    records+=("${fields[*]}")
                    fields=()
                    (( i += 2 ))
                else
                    fields+=("$field")
                    field=""
                    local last_idx=$(( ${#fields[@]} - 1 ))
                    if [[ "${fields[$last_idx]}" == *$'\r' ]]; then
                        fields[$last_idx]="${fields[$last_idx]%$'\r'}"
                    fi
                    records+=("${fields[*]}")
                    fields=()
                    (( i++ ))
                fi
            else
                field+="$ch"
                (( i++ ))
            fi
        fi
    done

    # Handle last record
    if [[ -n "$field" ]] || (( ${#fields[@]} > 0 )); then
        fields+=("$field")
        local last_idx=$(( ${#fields[@]} - 1 ))
        if [[ "${fields[$last_idx]}" == *$'\r' ]]; then
            fields[$last_idx]="${fields[$last_idx]%$'\r'}"
        fi
        records+=("${fields[*]}")
    fi

    # Now convert each record to TSV
    for rec in "${records[@]}"; do
        # Re-split the record by commas, respecting quotes
        local -a out_fields=()
        local cur_field=""
        local in_q=0
        local j=0
        local rlen=${#rec}
        while (( j < rlen )); do
            local c="${rec:$j:1}"
            if (( in_q )); then
                if [[ "$c" == '"' ]]; then
                    if (( j + 1 < rlen )) && [[ "${rec:$((j+1)):1}" == '"' ]]; then
                        cur_field+='"'
                        (( j += 2 ))
                    else
                        in_q=0
                        (( j++ ))
                    fi
                else
                    cur_field+="$c"
                    (( j++ ))
                fi
            else
                if [[ "$c" == '"' ]]; then
                    in_q=1
                    (( j++ ))
                elif [[ "$c" == ',' ]]; then
                    out_fields+=("$cur_field")
                    cur_field=""
                    (( j++ ))
                else
                    cur_field+="$c"
                    (( j++ ))
                fi
            fi
        done
        out_fields+=("$cur_field")

        # Join with tabs, escaping \t and \n in fields
        local line=""
        local first=1
        for f in "${out_fields[@]}"; do
            # Escape tabs and newlines in the field
            local escaped="${f//$'\t'/\\t}"
            escaped="${escaped//$'\n'/\\n}"
            escaped="${escaped//$'\r'/}"
            if (( first )); then
                line="$escaped"
                first=0
            else
                line+=$'\t'"$escaped"
            fi
        done
        printf '%s\n' "$line"
    done
}