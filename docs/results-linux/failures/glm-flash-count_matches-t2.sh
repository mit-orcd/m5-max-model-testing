count_matches() {
    local pattern="$1"
    local file="$2"
    grep -xF "$pattern" "$file" | wc -l
}