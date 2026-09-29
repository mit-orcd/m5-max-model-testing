count_matches() {
    local pattern="$1"
    local file="$2"
    grep -Fx "$pattern" "$file" 2>/dev/null | wc -l
}