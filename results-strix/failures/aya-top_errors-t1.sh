function top_errors() {
    local file="$1"
    local n="$2"
    local errors=()
    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            local signature="${line##*ERROR: }"
            errors+=("$signature")
        fi
    done < "$file"
    local counts=()
    for signature in "${errors[@]}"; do
        local count=$(echo "$signature" | tr -cd '#' | wc -c)
        counts+=("$count$signature")
    done
    local sorted_counts=($(sort -nr <<<"${counts[*]}"))
    local i=0
    while [[ $i -lt $n ]]; do
        local count="${sorted_counts[$i]%%#*}"
        local signature="${sorted_counts[$i]#*}"
        echo "$count $signature"
        ((i++))
    done
}