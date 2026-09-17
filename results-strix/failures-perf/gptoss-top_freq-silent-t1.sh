top_freq() {
    local file="$1"
    [ -z "$file" ] && return 1
    [ ! -f "$file" ] && return 1
    sort "$file" | uniq -c | sort -nr | head -n10
}