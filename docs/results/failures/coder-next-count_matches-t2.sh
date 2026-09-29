count_matches() {
    local pattern="$1"
    local file="$2"
    if [[ -z "$file" || ! -f "$file" ]]; then
        echo 0
        return 1
    fi
    grep -c -F "$pattern" "$file" 2>/dev/null || echo 0
}