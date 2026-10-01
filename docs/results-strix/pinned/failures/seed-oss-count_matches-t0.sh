count_matches() {
    local pattern="$1" file="$2"
    [ -r "$file" ] && [ -f "$file" ] && grep -Fc "$pattern" "$file" || echo 0
}