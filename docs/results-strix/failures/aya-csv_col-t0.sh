function csv_col() {
    local col=$1
    local header=$(head -n 1 < /dev/stdin)
    local cols=$(echo "$header" | tr ',' '\n')
    local idx=$(echo "$cols" | grep -m 1 "^$col$" | cut -d ':' -f 1)
    local val
    while read -r line; do
        val=$(echo "$line" | cut -d ',' -f "$idx")
        echo "$val"
    done
}