total_size() {
    local dir="$1"
    find "$dir" -type f -print0 | xargs -0 stat -f '%z' | awk '{s+=$1} END {print s+0}'
}