csv_col() {
    local name="$1"
    local header
    header=$(head -n 1)
    local col_index
    col_index=$(echo "$header" | tr ',' '\n' | grep -n "^$name$" | head -1 | cut -d: -f1)
    if [ -z "$col_index" ]; then
        echo "Column '$name' not found" >&2
        return 1
    fi
    tail -n +2 | while IFS=, read -ra cols; do
        echo "${cols[$((col_index - 1))]}"
    done
}