csv_col() {
    local column_name="$1"
    local column_index
    column_index=$(head -1 | awk -F, '{for(i=1;i<=NF;i++) if($i=="'"$column_name"'") {print i; exit}}')
    awk -v col="$column_index" -F, '{print $col}' "$2"
}