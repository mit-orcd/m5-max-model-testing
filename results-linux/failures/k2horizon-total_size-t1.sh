total_size() {
    local dir=$1
    find "$dir" -type f -print0 2>/dev/null | xargs -0 -I{} stat -f %z {} 2>/dev/null | awk '{s+=$1} END{print s+0}'
}