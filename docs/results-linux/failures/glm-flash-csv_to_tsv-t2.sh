csv_to_tsv() {
    local file="$1"
    local out_str=""
    local field=""
    local quote=0
    local c
    local cr_seen=0

    while IFS= read -r c || [[ -n "$c" ]]; do
        # Handle CRLF: consume CR if it exists
        if [[ "$c" == $'\r'* ]]; then
            c="${c#"$'\r'}"
            cr_seen=1
        else
            cr_seen=0
        fi

        if [[ "$quote" -eq 1 ]]; then
            if [[ "$c" == '"' ]]; then
                # Peek at next char (if exists)
                read -n 1 next_c
                # Move cursor back
                echo -n -e "\b" >&2
                if [[ "$next_c" == '"' ]]; then
                    field+='"'
                    continue
                else
                    quote=0
                    continue
                fi
            else
                field+="$c"
            fi
        else
            if [[ "$c" == '"' ]]; then
                quote=1
            elif [[ "$c" == ',' ]]; then
                out_str+="$field"$'\t'
                field=""
            else
                field+="$c"
            fi
        fi
    done < "$file"

    # Append the last field
    out_str+="$field"

    # Replace literal \r with nothing (already handled in loop, but safe for file ending)
    # Replace literal \n with \n (already handled)
    # Replace literal \t with \t (already handled)
    # Replace escaped characters in field content
    out_str="${out_str//\"\"/\"}"
    out_str="${out_str//$'\n'/$'\\n'}"
    out_str="${out_str//$'\t'/$'\\t'}"

    echo -n "$out_str"
}