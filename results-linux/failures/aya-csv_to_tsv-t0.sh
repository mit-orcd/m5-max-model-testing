csv_to_tsv() {
    local IFS=$'\n'
    local fields=()
    local -a record=()
    while IFS= read -r line; do
        if [[ $line == *"$"\* ]]; then
            # quoted field, remove quotes
            line=${line//\"/}
        fi
        fields+=("$line")
    done < "$1"
    IFS=$'\t'
    for field in "${fields[@]}"; do
        record+=("$field")
    done
    printf '%s\n' "${record[*]}"
}