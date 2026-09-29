top_freq() {
    local file="$1"
    sort "$file" | uniq -c | sort -nr | head -10 | awk '{print $1, $2}'
}