function top_freq() {
    local file="$1"
    local counts=()
    while IFS= read -r line; do
        counts+=("$line")
    done < "$file"
    local unique=($(printf "%s\n" "${counts[@]}" | sort -u))
    local freq=()
    for val in "${unique[@]}"; do
        local count=$(echo "${counts[@]}" | grep -c "$val")
        freq+=("$count $val")
    done
    printf "%s\n" "${freq[@]}" | sort -nr | head -n 10
}