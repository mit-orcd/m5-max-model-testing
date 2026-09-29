csv_to_tsv() {
    local file="$1"
    local line
    local field
    local in_quotes=0
    local output_line=""

    while IFS= read -r line; do
        line="${line%$'\r'}"
        while IFS= read -r field; do
            if [[ "$field" == *'"'* ]]; then
                if (( in_quotes )); then
                    in_quotes=0
                    output_line+=$'\t'
                else
                    in_quotes=1
                    output_line+="$field"$'\t'
                fi
            else
                if (( in_quotes )); then
                    output_line+="$field"
                else
                    output_line+="$field"$'\t'
                fi
            fi
        done < <(printf '%s\n' "$line" | tr ',' '\n')
        printf '%s\n' "$output_line"
        output_line=""
    done < "$file"
}