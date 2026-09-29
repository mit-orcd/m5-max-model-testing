function top_errors() {
    local file="$1"
    local n="$2"
    local err_counts=()
    local err_sigs=()

    if [[ ! -f "$file" ]]; then
        echo "File not found: $file"
        return 1
    fi

    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            local sig="${line##*ERROR:}"
            sig="${sig//[0-9]/#}"
            err_counts+=("$sig")
        fi
    done < "$file"

    for sig in "${err_counts[@]}"; do
        err_sigs+=("$sig")
    done

    local sorted_indices=($(for i in "${!err_counts[@]}"; do echo -en "${err_counts[$i]}\t$i\n"; done) | sort -nr | cut -f2 -d\ )

    for i in "${sorted_indices[@]}"; do
        echo "${err_counts[$i]} ${err_sigs[$i]}"
    done | head -n "$n"
}