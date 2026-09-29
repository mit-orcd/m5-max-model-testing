csv_col() {
    local col_name="$1"
    local col_index
    col_index=$(head -n1 | awk -v col="$col_name" -F, '{for(i=1;i<=NF;i++) if($i==col) {print i; exit}}')
    awk -v col="$col_index" -F, '{print $col}' 
}