count_matches() {
    grep -c "$1" "$2" || echo 0
}