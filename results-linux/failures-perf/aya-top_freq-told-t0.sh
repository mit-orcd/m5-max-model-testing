function top_freq() {
    local file="$1"
    local tmpfile=$(mktemp)
    local count
    local value
    local max_count=0

    # Read the file and store counts in an array
    IFS=$'\n' read -r -a counts <<< "$(sort -rn "$file" | awk -F ' ' '{print $1}')"

    # Print the 10 most frequent values
    for (( i=0; i<10; i++ )); do
        count=${counts[i]}
        value=$(sort -rn "$file" | awk -v count="$count" -F ' ' '{print $2}' | head -n "$count")
        echo "$count $value"
    done

    rm "$tmpfile"
}