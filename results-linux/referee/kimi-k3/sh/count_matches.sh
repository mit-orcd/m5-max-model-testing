count_matches() {
    grep -c -F "$1" "$2" || true
}
