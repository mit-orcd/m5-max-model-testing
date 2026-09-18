top_freq() {
    sort "$1" | uniq -c | sort -rn | head -10 | awk '{print $1, $2}'
}