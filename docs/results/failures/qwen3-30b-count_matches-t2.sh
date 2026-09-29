count_matches() {
    grep -F -x -c "$1" "$2"
}