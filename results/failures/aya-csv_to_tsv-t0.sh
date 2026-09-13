csv_to_tsv() {
    local IFS=$'\n'
    local field
    local -a fields
    local -a out

    while IFS= read -r line; do
        fields=()
        while IFS= read -r field; do
            fields+=("$field")
        done <<< "$line"
        out+=("${fields[*]:@/^\"\"/\\"$'\n'"/@/\\t/\\t}")
    done < "$1"
    printf '%s\n' "${out[*]}"
}