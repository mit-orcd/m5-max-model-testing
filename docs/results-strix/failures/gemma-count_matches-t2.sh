count_matches() {
    grep -F -c "$1" "$2" 2>/dev/null || echo 0
}