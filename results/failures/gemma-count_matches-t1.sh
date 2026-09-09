count_matches() {
    grep -F -c "$1" "$2" || echo 0
}