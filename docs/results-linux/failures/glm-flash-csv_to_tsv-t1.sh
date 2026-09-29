csv_to_tsv() {
    local in_file="$1"
    local state=0
    local current_field=""
    local output_line=""

    # Process the file character by character
    while IFS= read -r -n 1 char; do
        if [[ -z "$char" ]]; then
            # Handle end of file (last line may not have newline)
            process_output
            break
        fi

        case $state in
            0) # Outside field, expecting delimiter or quote or start of field
                if [[ "$char" == '"' ]]; then
                    state=1
                elif [[ "$char" == $'\t' || "$char" == ',' ]]; then
                    # Handle empty fields
                    current_field=""
                    state=0
                else
                    # Start of field
                    current_field="$char"
                    state=2
                fi
                ;;
            1) # Inside quotes
                if [[ "$char" == '"' ]]; then
                    state=3
                else
                    current_field+="$char"
                fi
                ;;
            2) # Inside field (not quoted)
                if [[ "$char" == $'\t' || "$char" == ',' ]]; then
                    state=0
                else
                    current_field+="$char"
                fi
                ;;
            3) # Closing quote
                if [[ "$char" == '"' ]]; then
                    # Escaped quote
                    current_field+='"'
                    state=1
                else
                    # End of quoted field
                    state=0
                fi
                ;;
        esac
    done < "$in_file"
}

process_output() {
    # Escape characters for TSV
    local escaped_field="${current_field//\\/\\\\}"
    escaped_field="${escaped_field//$'\n'/\\n}"
    escaped_field="${escaped_field//$'\r'/\\r}"
    escaped_field="${escaped_field//$'\t'/\\t}"
    
    # Handle potential empty last field
    if [[ -z "$current_field" ]]; then
        echo -e -n "$output_line"
    else
        echo -e -n "$output_line$escaped_field"
    fi
    
    # Print the line with a newline
    if [[ -n "$output_line" ]]; then
        echo ""
    fi
}

# Main execution logic
csv_to_tsv "$@"