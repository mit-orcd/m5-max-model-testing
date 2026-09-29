count_matches() {
    pattern="$1"
    file="$2"
    grep -c -F -- "$pattern" "$file" 2>/dev/null || echo 0
}