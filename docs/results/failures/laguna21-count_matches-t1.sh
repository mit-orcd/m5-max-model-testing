count_matches() {
    local pattern="$1"
    local file="$2"
    local count=$(grep -F -x "$pattern" "$file" | wc -l)
    echo "$count"
}