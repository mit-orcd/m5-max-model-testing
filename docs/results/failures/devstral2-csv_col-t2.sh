csv_col() {
    local -n name="$1"
    local header
    read -r header
    IFS=, read -ra fields <<< "$header"
    local -i col=0
    local -i found=0
    for i in "${!fields[@]}"; do
        if [[ "${fields[i]}" == *"$name"* ]]; then
            col=$i
            found=1
            break
        fi
    done
    if [[ $found -eq 0 ]]; then
        return 1
    fi
    while IFS=, read -ra row; do
        echo "${row[col]}"
    done
}