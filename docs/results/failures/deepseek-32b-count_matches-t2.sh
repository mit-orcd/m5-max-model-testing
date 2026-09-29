count_matches() {
    grep -Fc "$1" "$2" 2>/dev/null || echo 0
}