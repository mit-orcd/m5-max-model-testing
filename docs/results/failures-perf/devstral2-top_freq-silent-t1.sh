top_freq() {
    sort | uniq -c | sort -nr | awk '{print $1, $2}' | head -n 10
}