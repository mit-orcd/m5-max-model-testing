csv_to_tsv () {
    local IFS=$'\n'
    local input=()
    local line=()
    local field=()
    while IFS= read -r -a line; do
        input+=("$line")
    done < "$1"
    for line in "${input[@]}"; do
        IFS=$'\t' read -r -a field <<< "$line"
        printf '%s\n' "${field[*]}"
    done
}