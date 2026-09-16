csv_to_tsv() {
    local file="$1"
    local c
    local field=""
    local in_quotes=0
    local output=""
    
    # Read file, handling CRLF and LF
    while IFS= read -r -d '' c || [[ -n "$c" ]]; do
        # Handle CR: skip it (CRLF -> LF)
        [[ "$c" == $'\r' ]] && continue
        
        if (( in_quotes )); then
            case "$c" in
                '"')
                    # Check for escaped quote ""
                    if IFS= read -r -d '' next_c 2>/dev/null || [[ -n "$next_c" ]]; then
                        if [[ "$next_c" == '"' ]]; then
                            # Escaped quote: add one quote to field
                            field+='"'
                            c=""
                            continue
                        else
                            # End of quoted field
                            in_quotes=0
                            # Put back the character we read too far
                            printf -v _ "%s" "$c"
                            c="$next_c"
                        fi
                    else
                        # End of quoted field at EOF
                        in_quotes=0
                    fi
                    ;;
                $'\n')
                    # Replace newline with \n
                    field+=$'\\n'
                    ;;
                $'\t')
                    # Replace tab with \t
                    field+=$'\\t'
                    ;;
                *)
                    field+="$c"
                    ;;
            esac
        else
            case "$c" in
                '"')
                    in_quotes=1
                    ;;
                ',')
                    # Process field separator
                    # Escape tabs and newlines for output
                    local escaped_field="${field//$'\t'/\\t}"
                    escaped_field="${escaped_field//$'\n'/\\n}"
                    output+="$escaped_field"$'\t'
                    field=""
                    ;;
                $'\n')
                    # End of record
                    # Escape tabs and newlines for output
                    local escaped_field="${field//$'\t'/\\t}"
                    escaped_field="${escaped_field//$'\n'/\\n}"
                    output+="$escaped_field"$'\n'
                    field=""
                    ;;
                *)
                    field+="$c"
                    ;;
            esac
        fi
    done < "$file"
    
    # Handle last field if no trailing newline
    if [[ -n "$field" ]] || (( in_quotes )); then
        local escaped_field="${field//$'\t'/\\t}"
        escaped_field="${escaped_field//$'\n'/\\n}"
        output+="$escaped_field"$'\n'
    fi
    
    printf '%s' "$output"
}