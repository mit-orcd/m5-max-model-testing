count_matches() {
    local pattern="$1"
    local file="$2"
    local count
    count=$(grep -Fxc "$pattern" "$file" 2>/dev/null)
    echo "${count:-0}"
}