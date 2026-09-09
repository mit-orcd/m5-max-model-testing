csv_col() {
    local name=$1
    local header
    read header < /dev/stdin
    local i=0
    while IFS= read -r line; do
        local val=$(echo "$line" | cut -d, -f$i)
        if [[ $val == *"$name"* ]]; then
            echo "$val"
        fi
    done
}