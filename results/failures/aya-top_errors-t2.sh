function top_errors() {
    local logfile="$1"
    local n="$2"
    local error_ patterns=()
    local counts=()

    if [[ ! -f "$logfile" ]]; then
        echo "File not found: $logfile"
        return 1
    fi

    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            local signature="${line##*ERROR: }"
            local digits_removed="${signature//[0-9]/#}"
            error_patterns+=("$digits_removed")
        fi
    done < "$logfile"

    local unique_errors=($(sort -u <<<"${error_patterns[*]}")))

    for pattern in "${unique_errors[@]}"; do
        local count=$(grep -c "ERROR:$pattern" "$logfile")
        counts+=("$count")
    done

    local sorted_indices=($(for (( i=0; i<${#unique_errors[@]}; i++ )); do echo -en "${unique_errors[$i]} ${counts[$i]}\n"; done | sort -rn | cut -d' ' -f2))

    for i in "${sorted_indices[@]}"; do
        echo "${counts[$i]} ${unique_errors[$i]}"
    done | sort -rn | head -n "$n"
}