csv_to_tsv() {
    local file="$1"
    local state="field"
    local field=""
    local c prev_c
    local i len
    local -a fields=()
    
    # Read file byte by byte
    while IFS= read -r -d '' -n1 c || [[ -n "$c" ]]; do
        case "$state" in
            field)
                if [[ "$c" == '"' ]]; then
                    state="quoted"
                    field=""
                elif [[ "$c" == $'\t' || "$c" == $'\n' || "$c" == $'\r' ]]; then
                    # Unquoted tab/newline is record separator
                    if [[ "$c" == $'\r' ]]; then
                        # Check for CRLF
                        if IFS= read -r -d '' -n1 prev_c && [[ "$prev_c" == $'\n' ]]; then
                            : # CRLF, consume the LF
                        else
                            # Standalone CR (rare), treat as LF
                            prev_c=$'\n'
                        fi
                        c=$'\n'
                    fi
                    fields+=("$field")
                    field=""
                    if [[ "$c" == $'\n' ]]; then
                        # Output record
                        printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                        fields=()
                    fi
                elif [[ "$c" == ',' ]]; then
                    fields+=("$field")
                    field=""
                else
                    field+="$c"
                fi
                ;;
            quoted)
                if [[ "$c" == '"' ]]; then
                    # Could be end quote or escaped quote
                    if IFS= read -r -d '' -n1 prev_c && [[ "$prev_c" == '"' ]]; then
                        # Escaped quote ""
                        field+='"'
                    else
                        # End quote
                        state="field"
                        # Check if followed by comma or newline
                        if [[ "$prev_c" == ',' ]]; then
                            fields+=("$field")
                            field=""
                        elif [[ "$prev_c" == $'\n' ]]; then
                            fields+=("$field")
                            printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                            fields=()
                        elif [[ "$prev_c" == $'\r' ]]; then
                            # Check for CRLF
                            if IFS= read -r -d '' -n1 c && [[ "$c" == $'\n' ]]; then
                                fields+=("$field")
                                printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                                fields=()
                            else
                                # Standalone CR
                                fields+=("$field")
                                printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                                fields=()
                            fi
                        elif [[ -z "$prev_c" ]]; then
                            # EOF after quote
                            fields+=("$field")
                            printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                            fields=()
                        fi
                    fi
                else
                    field+="$c"
                fi
                ;;
        esac
    done < "$file"
    
    # Handle last field if no trailing newline
    if [[ "$state" == "field" && -n "$field" ]]; then
        fields+=("$field")
        printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
    elif [[ "$state" == "quoted" && -n "$field" ]]; then
        # Incomplete quoted field at EOF - output as is
        fields+=("$field")
        printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
    fi
}