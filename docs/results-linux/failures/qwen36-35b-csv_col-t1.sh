csv_col() {
    local col_name="$1"
    local header_line
    local col_index=-1

    # Read the header line
    IFS= read -r header_line

    # Find the column index by iterating through the header
    local IFS=','
    local -a headers
    read -r -a headers <<< "$header_line"

    for i in "${!headers[@]}"; do
        if [[ "${headers[$i]}" == "$col_name" ]]; then
            col_index=$((i + 1))
            break
        fi
    done

    # If column not found, exit silently or error (here we exit silently)
    if [[ $col_index -eq -1 ]]; then
        return 1
    fi

    # Read the rest of the CSV and print the matching column
    while IFS= read -r line; do
        # Skip empty lines if necessary, but standard CSV processing
        # We use awk for robustness in extracting the specific column
        :
    done

    # Actually, let's just pipe the remaining stdin to awk for simplicity and robustness
    # But we already consumed stdin in the read loop above.
    # So we need to handle the whole thing differently.
}