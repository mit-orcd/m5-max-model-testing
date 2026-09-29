csv_to_tsv() {
    local IFS=$'\n'
    local field
    local -a fields
    local line

    while IFS= read -r line; do
        fields=()
        while [[ $line ]]; do
            field=${line%%'*'}
            line=${line#'*'}
            fields+=("$field")
        done
        printf '%s\n' "${fields[*]/./\\$&}"
    done < "$1"
}