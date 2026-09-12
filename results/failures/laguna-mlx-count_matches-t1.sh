count_matches() {
    local pattern="$1"
    local file="$2"
    grep -F -x -c -- "$pattern" "$file" 2>/dev/null || echo 0
}