csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local output=""
    local c prev_c
    local -i i=0
    local -i len
    local line
    local -a lines=()
    
    # Read entire file, handling CRLF and LF
    while IFS= read -r -d '' -n 1 c || [[ -n "$c" ]]; do
        # Remove CR
        [[ "$c" == $'\r' ]] && continue
        
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$c" == '"' ]]; then
                # Check for escaped quote ""
                if [[ $((i+1)) -lt ${#line} ]] && [[ "${line:$((i+1)):1}" == '"' ]]; then
                    field+='"'
                    ((i++))
                else
                    in_quotes=0
                fi
            else
                case "$c" in
                    $'\n') field+='\n' ;;
                    $'\t') field+='\t' ;;
                    *) field+="$c" ;;
                esac
            fi
        else
            case "$c" in
                '"')
                    in_quotes=1
                    ;;
                ',')
                    # End of field
                    lines+=("$field")
                    field=""
                    ;;
                $'\n')
                    # End of record
                    lines+=("$field")
                    field=""
                    # Output TSV line
                    printf '%s\n' "$(IFS=$'\t'; echo "${lines[*]}")"
                    lines=()
                    ;;
                *)
                    field+="$c"
                    ;;
            esac
        fi
        ((i++))
    done < "$file"
    
    # Handle last record if no trailing newline
    if [[ -n "$field" ]] || [[ ${#lines[@]} -gt 0 ]]; then
        lines+=("$field")
        printf '%s\n' "$(IFS=$'\t'; echo "${lines[*]}")"
    fi
}