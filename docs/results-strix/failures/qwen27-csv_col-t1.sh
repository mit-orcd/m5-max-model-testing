csv_col() {
    local name="$1"
    local header col_index
    header=$(head -n1)
    col_index=$(echo "$header" | awk -F',' -v name="$name" '{
        for (i=1; i<=NF; i++) {
            if ($i == name) {
                print i
                exit
            }
        }
    }')
    if [[ -z "$col_index" ]]; then
        return 1
    fi
    tail -n +2 | awk -F',' -v idx="$col_index" '{print $idx}'
}