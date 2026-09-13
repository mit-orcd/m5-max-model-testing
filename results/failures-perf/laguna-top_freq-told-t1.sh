top_freq() {
    sort "$1" | uniq -c | sort -rn | head -10
}