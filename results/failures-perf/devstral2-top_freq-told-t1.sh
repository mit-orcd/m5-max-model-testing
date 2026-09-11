top_freq() {
    local file="$1"
    sort "$file" | uniq -c | sort -nr | head -n 10
}