count_matches() {
    pattern="$1"
    file="$2"
    grep -c "$pattern" "$file" || echo 0
}