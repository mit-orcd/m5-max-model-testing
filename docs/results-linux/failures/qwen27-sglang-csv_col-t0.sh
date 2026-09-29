csv_col() {
    local name="$1"
    local header
    IFS= read -r header
    local idx
    idx=$(awk -F',' -v col="$name" 'BEGIN{for(i=1;i<=NF;i++) if($i==col){print i; exit}}' <<< "$header")
    if [[ -z "$idx" ]]; then
        return 1
    fi
    awk -F',' -v idx="$idx" '{print $idx}'
}