function count_matches() {
    local pattern="$1"
    local file="$2"
    local count=0

    while IFS= read -r line; do
        if [[ "$line" == "$pattern"* ]]; then
            count=$((count + 1))
        fi
    done < "$file"

    echo "$count"
}