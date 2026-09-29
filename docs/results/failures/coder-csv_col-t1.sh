csv_col() {
    local col_name="$1"
    local col_index=$(head -n1 | tr ',' '\n' | grep -.indexOf "$col_name")
    awk -v col="$col_index" 'BEGIN {FS=","} {print $col}'
}