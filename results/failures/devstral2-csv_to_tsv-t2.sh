csv_to_tsv() {
    local file="$1"
    local line
    while IFS= read -r line || [[ -n "$line" ]]; do
        local escaped_line="${line//$'\r'/}"  # Remove CR
        local output_line="" in_quotes=false
        for (( i=0; i<${#escaped_line}; i++ )); do
            local c="${escaped_line:$i:1}"
            if [[ "$in_quotes" == false ]]; then
                if [[ "$c" == "\"" ]]; then
                    in_quotes=true
                elif [[ "$c" == "," ]]; then
                    output_line+=$'\t'
                elif [[ "$c" == $'\n' ]]; then
                    output_line+=$'\n'
                else
                    output_line+="$c"
                fi
            else
                if [[ "$c" == "\"" ]]; then
                    if [[ "${escaped_line:$((i+1)):1}" == "\"" ]]; then
                        output_line+=$'"'  # Escape double quote
                        i=$((i+1))
                    else
                        in_quotes=false
                    fi
                elif [[ "$c" == $'\n' ]]; then
                    output_line+=$'\\n'
                elif [[ "$c" == $'\t' ]]; then
                    output_line+=$'\\t'
                else
                    output_line+="$c"
                fi
            fi
        done
        if [[ "$in_quotes" == true ]]; then
            echo "Unclosed quote in input" >&2
            exit 1
        fi
        echo -n "$output_line" | sed 's/ *$//' | sed 's/^,/\t/' | sed 's/,$//'
        echo -e "\n"
    done < "$file"
}