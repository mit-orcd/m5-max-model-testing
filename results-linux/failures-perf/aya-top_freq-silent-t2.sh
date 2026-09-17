function top_freq() {
    local file="$1"
    local counts=$(awk '{print $0, " " ,NR}' "$file" | sort | awk '{print $2, $1}')
    local max=$(echo "$counts" | awk '{print $1}')
    local threshold=$(echo "$max" | awk -F. -v max=1 '{print max - 1}')
    local i=1
    for count in $counts; do
        if [[ $i -eq 1 ]]; then
            echo "$count"
        elif [[ $count -ge $threshold ]]; then
            echo "$count"
        fi
        i=$((i+1))
    done | awk '{print $1, $2}'
}