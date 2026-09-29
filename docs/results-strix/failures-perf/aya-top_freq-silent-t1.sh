function top_freq() {
    local file="$1"
    local count=()
    local value=()
    local max_count=0

    while IFS= read -r line; do
        if [[ $line ]]; then
            local this_count=$(echo "$line" | wc -l)
            count+=("$this_count")
            value+=("$line")
            max_count=$(($max_count > $this_count ? $max_count : $this_count))
        fi
    done < "$file"

    local -t result=()
    for i in "${!count[@]}"; do
        result+=("$((count[i] / max_count * 100)) % ${#value[i]} ${value[i]}")
    done

    sort -nr -k1,1 <<< "${result[@]}" | head -n 10
}