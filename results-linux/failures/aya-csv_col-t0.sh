function csv_col() {
    local col=$1
    local header=$(head -n 1 < /dev/stdin)
    local cols=$(echo "$header" | tr ',' '\n' | wc -l)
    local i=0
    while IFS= read -r line; do
        local val=$(echo "$line" | awk -v col=$col -v header=$header '
            $1 == col { print $NF }
            $1 == header { for (i=2; i<=NF; i++) printf "%s,", $i; print "" }
        ')
        if [[ $val ]]; then
            echo "$val"
        fi
    done
}