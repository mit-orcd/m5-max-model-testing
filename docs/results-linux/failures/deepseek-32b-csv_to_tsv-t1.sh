csv_to_tsv() {
    local file=$1
    local in_quote=false
    local current_field=""
    local fields=()
    local acc_line=""
    local line

    while IFS= read -r line; do
        line=${line//$'\r'}
        acc_line+=$line
        if [[ $in_quote == false ]]; then
            while IFS= read -r -d , field < <(printf '%s' "$acc_line"); do
                fields+=("$field")
                acc_line=${acc_line#"$field,"}
                if [[ $field == *"\"* ]]; then
                    in_quote=true
                fi
            done
            if [[ $acc_line ]]; then
                fields+=("$acc_line")
                acc_line=""
            fi
        else
            while IFS= read -r -d \" field < <(printf '%s' "$acc_line"); do
                fields+=("${field//\"}")
                acc_line=${acc_line#"$field\""}
                in_quote=false
            done
            if [[ $acc_line ]]; then
                acc_line=${acc_line%\"}
                fields+=("$acc_line")
                acc_line=""
                in_quote=false
            fi
        fi
        if [[ $in_quote == false ]]; then
            for i in "${!fields[@]}"; do
                fields[$i]=${fields[$i]//[$'\t$'\n]/\\$BASH_REMATCH}
            done
            printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
            fields=()
        fi
    done < "$file"
    if [[ $in_quote == false && ${#fields[@]} -gt 0 ]]; then
        for i in "${!fields[@]}"; do
            fields[$i]=${fields[$i]//[$'\t$'\n]/\\$BASH_REMATCH}
        done
        printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
    fi
}