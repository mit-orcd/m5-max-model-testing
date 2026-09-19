top_freq() {
    local file="$1"
    local counts=()
    local max_count=0
    local max_index=0
    local temp_count=0
    local temp_value=""
    local i=0
    local j=0
    local k=0

    # Count frequencies
    while IFS= read -r line || [[ -n "$line" ]]; do
        counts+=("$line")
    done < "$file"

    local n=${#counts[@]}

    # Sort frequencies using a simple bubble sort
    for ((i = 0; i < n - 1; i++)); do
        for ((j = 0; j < n - i - 1; j++)); do
            if [[ ${counts[j]} < ${counts[j+1]} ]]; then
                temp_value="${counts[j]}"
                counts[j]="${counts[j+1]}"
                counts[j+1]="$temp_value"
            fi
        done
    done

    # Print top 10
    for ((i = 0; i < 10; i++)); do
        if [[ i -lt n ]]; then
            echo "$((n - i)) ${counts[i]}"
        fi
    done
}