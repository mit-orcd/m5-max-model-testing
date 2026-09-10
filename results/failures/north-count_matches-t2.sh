count_matches() {
    pattern="$1"
    file="$2"
    grep -F "$pattern" "$file" | wc -l
}