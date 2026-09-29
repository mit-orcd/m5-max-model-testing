top_freq() {
    sort "$1" | uniq -c | sort -k1,1nr -k2,2 | head -n 10
}