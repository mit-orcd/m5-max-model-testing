csv_to_tsv() {
    local file="$1"
    local line
    local in_quotes=0
    local field
    local output
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Remove CR if present (CRLF handling)
        line="${line%$'\r'}"
        local i=0
        local len=${#line}
        output=""
        field=""
        while [[ $i -lt $len ]]; do
            char="${line:i:1}"
            if [[ $in_quotes -eq 1 ]]; then
                if [[ "$char" == "\"" ]]; then
                    # Check for escaped quote
                    if [[ $i -lt $len-1 && "${line:i+1:1}" == "\"" ]]; then
                        field+='"'
                        ((i+=2))
                    else
                        in_quotes=0
                        ((i+=1))
                    fi
                else
                    # Handle tab and newline in quoted field
                    if [[ "$char" == $'\t' ]]; then
                        field+="\\t"
                    elif [[ "$char" == $'\n' ]]; then
                        field+="\\n"
                    else
                        field+="$char"
                    fi
                    ((i+=1))
                fi
            else
                if [[ "$char" == "\"" ]]; then
                    in_quotes=1
                    ((i+=1))
                elif [[ "$char" == "," ]]; then
                    # Output field
                    if [[ -n "$output" ]]; then
                        output+="$field\t"
                    else
                        output+="$field\t"
                    fi
                    field=""
                    ((i+=1))
                else
                    field+="$char"
                    ((i+=1))
                fi
            fi
        done
        # Add last field
        if [[ -n "$field" ]]; then
            if [[ -n "$output" ]]; then
                output+="$field"
            else
                output+="$field"
            fi
        fi
        # Output the line
        echo "$output"
    done < "$file"
    echo
}