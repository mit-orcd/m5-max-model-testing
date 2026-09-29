csv_col() {
    local -r name="$1"
    local -r header=$(head -n 1)
    local -r col_index=$(echo "$header" | tr ',' '\n' | grep -n "^${name}$" | cut -d: -f1)
    tail -n +2 | while IFS=, read -r -a row; do
        if [ "$col_index" -le "${#row[@]}" ]; then
            echo "${row[$((col_index - 1))]}"
        fi
    done
}