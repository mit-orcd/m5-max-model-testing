count_matches() {
    grep -F "$1" "$2" | wc -l
}