csv_to_tsv() {
    local file="$1"
    local IFS=$'\n'
    local line field state inquote escape output
    local -a fields
    state=0  # 0: normal, 1: in quote
    escape=0
    while IFS= read -r line || [[ -n $line ]]; do
        # Remove trailing CR if present
        line="${line%$'\r'}"
        fields=()
        field=""
        inquote=0
        escape=0
        for (( i=0; i<${#line}; i++ )); do
            c="${line:i:1}"
            if [[ $escape -eq 1 ]]; then
                if [[ $c == '"' ]]; then
                    field+='"'
                else
                    field+='\\'"$c"  # Should not happen per RFC-4180, but handle
                fi
                escape=0
                continue
            fi
            if [[ $c == '"' ]]; then
                if [[ $inquote -eq 1 ]]; then
                    # Check for doubled quote
                    if [[ $((i+1)) -lt ${#line} && "${line:i+1:1}" == '"' ]]; then
                        field+='"'
                        ((i++))
                    else
                        inquote=0
                    fi
                else
                    inquote=1
                fi
            elif [[ $c == ',' && $inquote -eq 0 ]]; then
                fields+=("$field")
                field=""
            elif [[ $c == $'\t' && $inquote -eq 1 ]]; then
                field+=$'\\t'
            elif [[ $c == $'\n' && $inquote -eq 1 ]]; then
                field+=$'\\n'
            else
                field+="$c"
            fi
        done
        fields+=("$field")
        # Join fields with tab
        output=""
        for f in "${fields[@]}"; do
            if [[ -z $output ]]; then
                output="$f"
            else
                output+=$'\t'"$f"
            fi
        done
        printf '%s\n' "$output"
    done < "$file"
}