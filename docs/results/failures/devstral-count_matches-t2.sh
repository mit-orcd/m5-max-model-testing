count_matches() {
    local pattern="$1"
    local file="$2"
    grep -Fc "$pattern" "$file" || echo 0
}