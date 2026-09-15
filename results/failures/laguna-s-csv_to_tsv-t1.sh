csv_to_tsv() {
    local state=field line_buffer="" field_buffer="" char prev_char=""
    local record=()
    
    while IFS= read -r -N 1 char || [[ -n "$char" ]]; do
        if [[ "$char" == $'\r' ]]; then
            continue
        fi
        
        case "$state" in
            field)
                case "$char" in
                    '"')
                        if [[ -z "$field_buffer" ]]; then
                            state=quoted
                        elif [[ "${field_buffer: -1}" == '"' ]]; then
                            field_buffer="${field_buffer%?}\""
                        else
                            field_buffer+="$char"
                        fi
                        ;;
                    ',')
                        record+=("$field_buffer")
                        field_buffer=""
                        ;;
                    $'\n')
                        record+=("$field_buffer")
                        field_buffer=""
                        printf '%s\n' "$(IFS=$'\t'; printf '%s' "${record[*]}")" | sed 's/\\t/\\t/g; s/\\n/\\n/g'
                        record=()
                        ;;
                    *)
                        field_buffer+="$char"
                        ;;
                esac
                ;;
            quoted)
                case "$char" in
                    '"')
                        if [[ -z "$field_buffer" ]]; then
                            state=field
                        else
                            field_buffer+="$char"
                        fi
                        ;;
                    *)
                        field_buffer+="$char"
                        ;;
                esac
                ;;
        esac
    done < "$1"

    if [[ -n "$field_buffer" || ${#record[@]} -gt 0 ]]; then
        record+=("$field_buffer")
        printf '%s\n' "$(IFS=$'\t'; printf '%s' "${record[*]}")" | sed 's/\\t/\\t/g; s/\\n/\\n/g'
    fi
}