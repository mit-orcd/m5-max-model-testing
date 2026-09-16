csv_col() {
    local name="$1"
    local -i col_num
    local header line

    # Read header line and find column index
    IFS= read -r header
    col_num=$(awk -F, -v name="$name" 'BEGIN{split(name,a,","); for(i in a) if($0~a[i]) print index($0,a[i])}' <<< "$header" | sort -n | head -n1)

    # Check if column exists
    if [[ -z $col_num || $col_num -lt 1 ]]; then
        return 1
    fi

    # Print matching column from all lines (including header)
    awk -F, -v col="$col_num" 'NR==1 || NR>1 {print $col}'
}