count_matches() {
    grep -c -F "$1" "$2" 2>/dev/null || echo 0
}