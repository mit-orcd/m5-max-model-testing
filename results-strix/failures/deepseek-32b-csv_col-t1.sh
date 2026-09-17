csv_col() {
    local name="$1"
    local header=$(head -n 1)
    local index=0
    IFS=',' read -ra cols <<< "$header"
    for i in "${!cols[@]}"; do
        if [[ "${cols[$i]}" == "$name" ]]; then
            index=$((i + 1))
            break
        fi
    done
    if [[ $index -eq 0 ]]; then
        return
    fi
    tail -n +2 | cut -d ',' -f "$index"
}