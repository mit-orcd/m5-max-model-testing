csv_col() {
    local -r name="$1"
    local header line
    read -r header || return 1
    while IFS=, read -r -a fields; do
        for i in "${!fields[@]}"; do
            if [[ "${header/,/}" == "${fields[i],/}" ]]; then
                echo "${fields[i]}"
                break
            fi
        done
    done
}