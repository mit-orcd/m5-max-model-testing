csv_to_tsv() {
    local file="$1"
    local escaped=0
    local in_quotes=0
    local output_line=""
    local current_field=""
    local last_char=""

    while IFS= read -r -n1 char || [ -n "$char" ]; do
        if [ "$in_quotes" -eq 1 ]; then
            case "$char" in
                '"')
                    if [ -n "$last_char" ] && [ "$last_char" = '"' ]; then
                        current_field="${current_field%?}"  # Remove the last quote
                        last_char=""
                    else
                        in_quotes=0
                        last_char=""
                    fi
                    ;;
                $'\n')
                    current_field+="\\n"
                    last_char=""
                    ;;
                $'\t')
                    current_field+="\\t"
                    last_char=""
                    ;;
                *)
                    current_field+="$char"
                    last_char="$char"
                    ;;
            esac
        else
            case "$char" in
                '"')
                    in_quotes=1
                    last_char=""
                    ;;
                $'\n')
                    output_line+="$current_field$( [ -n "$output_line" ] && echo -n $'\t' )"
                    current_field=""
                    if [ -z "$output_line" ] && [ -z "$current_field" ]; then
                        echo -e "$output_line"  # Output empty line
                    else
                        echo -e "$output_line"
                    fi
                    output_line=""
                    last_char=""
                    ;;
                ',')
                    output_line+="$current_field$( [ -n "$output_line" ] && echo -n $'\t' )"
                    current_field=""
                    last_char=""
                    ;;
                *)
                    current_field+="$char"
                    last_char="$char"
                    ;;
            esac
        fi
    done < "$file"

    # Handle the last field
    if [ -n "$current_field" ]; then
        output_line+="$current_field$( [ -n "$output_line" ] && echo -n $'\t' )"
    fi

    # Output the last line if not empty
    if [ -n "$output_line" ]; then
        echo -e "$output_line"
    fi
}