function csv_col() {
    local name=$1
    local header=$(head -1)
    local cols=$(echo "$header" | tr ',' '\n')
    local col_num=$(echo "$cols" | grep -n "$name" | cut -d: -f1)
    local col_val=$(echo "$header" | cut -d',' -f$col_num)
    local lines=$(wc -l < /dev/stdin)
    local i=2
    while read line; do
        local val=$(echo "$line" | cut -d',' -f$col_num)
        echo "$val"
        i=$((i+1))
    done
}