function csv_col() {
    local col=$1
    local header=$(head -n 1 < /dev/stdin)
    local cols=$(echo "$header" | tr ',' '\n')
    local col_idx=$(echo "$cols" | grep -m 1 -n "$col" | cut -d: -f1)
    local col_val=$(echo "$header" | cut -d',' -f$col_idx)

    while IFS=, read -r line; do
        local val=$(echo "$line" | cut -d',' -f$col_idx)
        echo "$val"
    done
}