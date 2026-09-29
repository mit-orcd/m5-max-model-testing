csv_col() {
    local name="$1"
    local col_num
    IFS=',' read -ra headers
    for i in "${!headers[@]}"; do
        if [[ "${headers[i]}" == "$name" ]]; then
            col_num=$((i + 1))
            break
        fi
    done
    if [[ -z "${col_num:-}" ]]; then
        return 1
    fi
    awk -F',' -v col="$col_num" 'NR > 1 { print $col }'
}