function top_freq() {
    local file="$1"
    local counts=()
    while IFS= read -r line; do
        counts+=("$line")
    done < "$file"
    local unique=($(printf '%s\n' "${counts[@]}" | sort -u))
    local freq=()
    for val in "${unique[@]}"; do
        local count=$(printf '%s\n' "${counts[@]}" | grep -c "$val")
        freq+=("$count $val")
    done
    local sorted=($(for val in "${freq[@]}"; do echo "$val"; done | sort -nr))
    for val in "${sorted[@]}"; do
        echo "$val"
    done
}