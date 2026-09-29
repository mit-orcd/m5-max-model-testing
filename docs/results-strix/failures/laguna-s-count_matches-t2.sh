count_matches() {
    grep -c -- "$1" "$2" 2>/dev/null || echo 0
}