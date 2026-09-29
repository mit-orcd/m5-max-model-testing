count_matches() {
    grep -cF -- "$1" "$2" 2>/dev/null || echo 0
}