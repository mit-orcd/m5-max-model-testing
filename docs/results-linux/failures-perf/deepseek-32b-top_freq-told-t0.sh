top_freq() {
    sort < "$1" | uniq -c | sort -nr | head -n 10
}