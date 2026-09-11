top_freq() {
    sort | uniq -c | sort -nr | head -n 10
}