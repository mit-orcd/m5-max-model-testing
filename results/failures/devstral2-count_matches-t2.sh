count_matches() {
    local pattern="$1"
    local file="$2"
    grep -c "$pattern" "$file" || echo 0
}