count_matches() {
    grep -F -c -x -e "$1" "$2"
}