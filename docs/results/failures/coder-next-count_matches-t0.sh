count_matches() {
    local pattern="$1"
    local file="$2"
    grep -F "$pattern" "$file" 2>/dev/null | wc -l
}