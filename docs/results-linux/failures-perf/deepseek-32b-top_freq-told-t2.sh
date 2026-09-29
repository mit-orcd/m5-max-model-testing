top_freq() {
    sort "$1" | uniq -c | sort -k1,1nr | head -n 10
}