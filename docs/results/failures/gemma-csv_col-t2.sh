csv_col() {
    local target_name="$1"
    local col_idx=0

    # Read the header first
    IFS=',' read -r -a headers

    # Find the column index
    for i in "${!headers[@]}"; do
        if [[ "${headers[$i]}" == "$target_name" ]]; then
            col_idx=$((i + 1))
            break
        fi
    done

    if [[ $col_idx -eq 0 ]]; then
        return 1
    fi

    # Use awk to print the specific column from stdin
    awk -F',' -v col="$col_idx" 'NR > 1 {print $col}'
}