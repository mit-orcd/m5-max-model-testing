csv_col() {
    local col_name="$1"
    local header
    IFS= read -r header
    if [ -z "$header" ]; then
        return 0
    fi

    # Find column index (0-based)
    local col_idx=-1
    local IFS=','
    local i=0
    for h in $header; do
        # Trim whitespace and quotes if necessary
        h=$(echo "$h" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//;s/^"//;s/"$//')
        if [ "$h" = "$col_name" ]; then
            col_idx=$i
            break
        fi
        ((i++))
    done

    if [ $col_idx -eq -1 ]; then
        return 1
    fi

    # Process remaining lines
    local line
    while IFS= read -r line; do
        if [ -z "$line" ]; then
            continue
        fi
        
        # Use awk to extract the column, handling CSV properly is complex, 
        # but for a standard bash one-liner we can use awk with comma delimiter.
        # This assumes no commas within quoted fields for simplicity, 
        # or uses a basic split. For robust CSV, awk with FPAT is better (GNU awk).
        echo "$line" | awk -v col="$((col_idx + 1))" 'BEGIN{FPAT="([^,]*)|(\"[^\"]*\")"} {print $col}'
    done
}