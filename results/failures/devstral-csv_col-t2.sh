csv_col() {
    local -r name="$1"
    local -r header=$(head -n 1)
    local -r index=$(echo "$header" | tr ',' '\n' | grep -n "$name" | head -1 | cut -d: -f1)
    if [ -z "$index" ]; then
        return 1
    fi
    tail -n +2 | while IFS= read -r line; do
        IFS=, read -ra cols <<< "$line"
        echo "${cols[$index-1]}"
    done
}