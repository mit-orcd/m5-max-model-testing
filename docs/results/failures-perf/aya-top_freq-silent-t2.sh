top_freq() {
    local file="$1"
    local counts=()
    local -A count_index
    local -A values
    local val
    local count
    local i=0

    if [[ ! -f "$file" ]]; then
        echo "Error: '$file' is not a file." >&2
        return 1
    fi

    while IFS= read -r val; do
        count=$(echo -n "$val" | base64 --decode-file /dev/stdin | wc -l)
        counts+=("$count")
        count_index+=([$count]=$i)
        values+=("$val")
        i=$((i+1))
    done < "$file"

    local max_count=${counts[0]}
    local -a top_counts=("${counts[@]}")
    local -a top_values=("${values[@]}")

    for ((i=0; i<${#counts[@]}; i++)); do
        if [[ ${counts[i]} -eq $max_count ]]; then
            top_counts+=(${counts[i]})
            top_values+=("${values[i]}")
        fi
    done

    local -a sorted_indices=("${!count_index[@]}")
    local -a sorted_counts=("${!top_counts[@]}")
    local -a sorted_values=("${!top_values[@]}")

    for ((i=0; i<10; i++)); do
        count=${sorted_counts[i]}
        val=${sorted_values[i]}
        echo "$count $val"
    done
}