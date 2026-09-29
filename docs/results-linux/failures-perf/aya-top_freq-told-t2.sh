function top_freq() {
    local file="$1"
    local tmpfile=$(mktemp)
    local count
    local value
    local max_count=0

    # Read the file and store counts in an array
    IFS=$'\n'
    read -r -a counts <<< "$(sort -t' ' -k2,2n -r "$file")"
    IFS=$'\n'

    # Print the 10 most frequent values
    for (( i=0; i<10; i++ )); do
        count=${counts[i]}
        value=${counts[i+1]}
        if [[ $count -gt $max_count ]]; then
            max_count=$count
        fi
        echo "$count $value"
    done

    # Clean up temporary file
    rm "$tmpfile"
}